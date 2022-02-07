
#include "common_application.h"
#include "input/image/file_input.h"
#include "input/image/minicap_input.h"
#include "input/image/scrcpy_input.h"
#include "input/request/http_request.h"
#include "ocr/paddle_ocr.h"
#include "output/notify//miao_notify.h"
#include "output/operation/dummy_operation.h"
#include "output/operation/minitouch_operation.h"
#include "output/operation/scrcpy_operation.h"
#include "parser/yaml_parser.h"
#include "player/common_player.h"
#include "utils/image_utils.h"
#include "utils/log.h"
#include "utils/util_functions.h"
#include <filesystem>
#include <yaml-cpp/yaml.h>

CommonApplication::CommonApplication(const std::string &config_fname) {
    status_ = ApplicationStatus::Stopped;
    config_abs_path_ = std::filesystem::absolute(config_fname).string();
}

bool CommonApplication::Init() {
    YAML::Node config_yaml(YAML::LoadFile(config_abs_path_));
    return InitWithYaml(config_yaml);
}

void CommonApplication::Start() {
    LOG_INFO("Application running");
    status_ = ApplicationStatus::Running;

    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        con_.wait(lock, [this] {
            return status_ == ApplicationStatus::Running ||
                   status_ == ApplicationStatus::Stopped;
        });
        if (status_ == ApplicationStatus::Stopped) {
            LOG_INFO("Application stoped");
            break;
        }
        lock.unlock();

        TimeLog image_time_log("Image source");
        Image image = source_->GetOneFrame();
        if (image.buffer.empty()) {
            LOG_INFO("Image is empty");
            continue;
        }
        image_time_log.Tok();

        TimeLog ocr_time_log("OCR");
        std::vector<TextBox> text_boxes = ocr_->Detect(image);
        if (text_boxes.empty()) {
            LOG_INFO("Text is empty");
            continue;
        }
        ocr_time_log.Tok();

        std::vector<PlayOperation> play_operations =
            player_->Play(image, {}, text_boxes);
        if (!play_operations.empty()) {
            TimeLog operation_time_log("Operation");
            for (const PlayOperation &play_operation : play_operations) {
                if (play_operation.type == PlayOperationType::SCREEN_CLICK) {
                    operation_->Click(play_operation.click.x,
                                      play_operation.click.y);
                } else if (play_operation.type == PlayOperationType::SLEEP) {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(play_operation.sleep_ms));
                }
            }
            operation_time_log.Tok();

            if (interval_ms_ > 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(interval_ms_));
            }
        } else {
            LOG_INFO("Operation is empty");
        }

        if (player_->GameOver()) {
            LOG_INFO("Application is over");
            status_ = ApplicationStatus::Over;
            if (notify_) {
                LOG_INFO("Notify %s over event", player_->GetMode().c_str());
                if (!notify_->Notify("Mode " + player_->GetMode() +
                                     " is over")) {
                    LOG_ERROR("Notify failed.");
                }
            }
        }
    }
}

void CommonApplication::Pause() {
    LOG_INFO("Application pause");
    std::lock_guard<std::mutex> lock(mutex_);
    status_ = ApplicationStatus::Pausing;
}

void CommonApplication::Continue() {
    LOG_INFO("Application continue");
    std::lock_guard<std::mutex> lock(mutex_);
    player_->GameContinue();
    status_ = ApplicationStatus::Running;
    con_.notify_one();
}

void CommonApplication::Stop() {
    LOG_INFO("Application stop");
    std::lock_guard<std::mutex> lock(mutex_);
    status_ = ApplicationStatus::Stopped;
    con_.notify_one();
}

bool CommonApplication::SetPlayer(const std::string &player_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &player : players_) {
        if (player->Name() == player_name) {
            player_ = player;
            return true;
        }
    }
    LOG_ERROR("Unknown player %s", player_name.c_str());
    return false;
}

std::string CommonApplication::GetPlayer() {
    std::lock_guard<std::mutex> lock(mutex_);
    return player_->Name();
}

bool CommonApplication::SetMode(const std::string &mode_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    return player_->SetMode(mode_name);
}

std::string CommonApplication::GetMode() {
    std::lock_guard<std::mutex> lock(mutex_);
    return player_->GetMode();
}

bool CommonApplication::InitWithYaml(const YAML::Node &yaml) {
    // Image source
    const YAML::Node &source_yaml(yaml["source"]);
    std::string source_type = source_yaml["type"].as<std::string>();
    if (source_type == "minicap") {
        auto server_info(GetServerInfo(source_yaml));
        source_.reset(new MinicapInput(std::get<0>(server_info),
                                       std::get<1>(server_info)));
    } else if (source_type == "file") {
        source_.reset(new FileImageInput(
            source_yaml["file_name"].as<std::string>(), ImageFormat::JPEG));
    } else if (source_type == "scrcpy") {
        auto server_info(GetServerInfo(source_yaml));
        source_.reset(new ScrcpyInput(std::get<0>(server_info),
                                      std::get<1>(server_info)));
    }
    if (!source_ || !source_->Init()) {
        LOG_ERROR("source init failed");
        return false;
    }
    LOG_INFO("Image source init success");
    Image image = source_->GetOneFrame();
    if (image.buffer.empty()) {
        LOG_ERROR("Image get frame failed");
        return false;
    }
    FrameSize frame_size = ImageSize(image);

    // OCR
    const YAML::Node &ocr_yaml(yaml["ocr"]);
    if (ocr_yaml["type"].as<std::string>() == "paddleocr") {
        auto server_info = GetServerInfo(ocr_yaml);
        if (ocr_yaml["recv_timeout"].IsDefined()) {
            ocr_.reset(new PaddleOcr(std::get<0>(server_info),
                                     std::get<1>(server_info),
                                     ocr_yaml["recv_timeout"].as<int>()));
        } else {
            ocr_.reset(new PaddleOcr(std::get<0>(server_info),
                                     std::get<1>(server_info)));
        }
    }
    if (!ocr_ || !ocr_->Init()) {
        LOG_ERROR("ocr init failed");
        return false;
    }
    LOG_INFO("OCR init success");

    // Player
    const YAML::Node &players_yaml(yaml["players"]);
    for (const auto &player_yaml : players_yaml) {
        std::filesystem::path player_config_path =
            std::filesystem::path(config_abs_path_).parent_path();
        player_config_path /= player_yaml["include"].as<std::string>();

        std::shared_ptr<IPlayer> player(
            CreatePlayer(player_config_path.string()));
        if (!player || !player->Init()) {
            LOG_ERROR("Player %s init failed", player->Name().c_str());
            continue;
        }
        LOG_INFO("Player %s init success", player->Name().c_str());
        players_.push_back(player);
    }
    if (!players_.empty()) {
        player_ = players_[0];
    } else {
        LOG_ERROR("Players is empty");
    }

    // Operation
    const YAML::Node &operation_yaml = yaml["operation"];
    std::string operation_type = operation_yaml["type"].as<std::string>();
    if (operation_type == "minitouch") {
        auto server_info = GetServerInfo(operation_yaml);
        int orientation = frame_size.first > frame_size.second ? 90 : 0;
        operation_.reset(new MinitouchOperation(
            std::get<0>(server_info), std::get<1>(server_info),
            frame_size.first, frame_size.second, orientation));
    } else if (operation_type == "dummy") {
        operation_.reset(new DummyOperation());
    } else if (operation_type == "scrcpy") {
        auto server_info = GetServerInfo(operation_yaml);
        operation_.reset(new ScrcpyOperation(std::get<0>(server_info),
                                             std::get<1>(server_info)));
    }
    if (!operation_ || !operation_->Init()) {
        LOG_ERROR("operation init failed");
        return false;
    }
    LOG_INFO("Operation init success");

    // Request
    const YAML::Node &request_yaml = yaml["request"];
    if (request_yaml.IsDefined()) {
        std::string request_type = request_yaml["type"].as<std::string>();
        if (request_type == "http") {
            auto server_info = GetServerInfo(request_yaml);
            request_.reset(new HttpRequest(std::get<0>(server_info),
                                           std::get<1>(server_info)));
        }
        if (!request_ || !request_->Init()) {
            request_.reset();
            LOG_ERROR("Request init failed");
        } else {
            request_->SetCallback(
                "/player/name", RequestOperation::Query,
                std::bind(&CommonApplication::QueryCurrentPlayerCallback, this,
                          std::placeholders::_1, std::placeholders::_2));

            request_->SetCallback(
                "/player/name", RequestOperation::Replace,
                std::bind(&CommonApplication::ReplaceCurrentPlayerCallback,
                          this, std::placeholders::_1, std::placeholders::_2));

            request_->SetCallback(
                "/player/mode/name", RequestOperation::Query,
                std::bind(&CommonApplication::QueryCurrentModeCallback, this,
                          std::placeholders::_1, std::placeholders::_2));

            request_->SetCallback(
                "/player/mode/name", RequestOperation::Replace,
                std::bind(&CommonApplication::ReplaceCurrentModeCallback, this,
                          std::placeholders::_1, std::placeholders::_2));

            request_->SetCallback(
                "/status", RequestOperation::Query,
                std::bind(&CommonApplication::QueryStatusCallback, this,
                          std::placeholders::_1, std::placeholders::_2));

            request_->SetCallback(
                "/status", RequestOperation::Replace,
                std::bind(&CommonApplication::ReplaceStatusCallback, this,
                          std::placeholders::_1, std::placeholders::_2));
        }
    } else {
        LOG_INFO("Request is not defined");
    }

    // Notify
    const YAML::Node &notify_yaml = yaml["notify"];
    if (notify_yaml.IsDefined()) {
        std::string notify_type = notify_yaml["type"].as<std::string>();
        if (notify_type == "miao") {
            notify_.reset(
                new MiaoNotify(notify_yaml["miao_key"].as<std::string>()));
        }
        if (!notify_ || !notify_->Init()) {
            notify_.reset();
            LOG_ERROR("Notify init failed");
        }
    } else {
        LOG_INFO("Notify is not defined");
    }

    // Application
    const YAML::Node &app_yaml = yaml["application"];
    if (app_yaml.IsDefined()) {
        interval_ms_ = app_yaml["interval"].IsDefined()
                           ? app_yaml["interval"].as<int>()
                           : 0;
    }

    return true;
}

IPlayer *CommonApplication::CreatePlayer(const std::string &config_path) {
    LOG_INFO("Load player %s", config_path.c_str());
    IPlayer *ret = nullptr;
    YAML::Node player_yaml = YAML::LoadFile(config_path);
    std::string player_name = player_yaml["name"].as<std::string>();
    if (player_yaml["type"].as<std::string>() == "common") {
        std::vector<PageConfig> page_configs;
        for (const auto &page_yaml : player_yaml["pages"]) {
            PageConfig page_config;
            page_config.name = page_yaml["name"].as<std::string>();

            for (auto &page_condition_yaml : page_yaml["conditions"]) {
                PageKeyElement key_element =
                    GetKeyElementConfig(page_condition_yaml);
                page_config.key_elements.push_back(key_element);
            }
            page_configs.push_back(page_config);
        }

        std::vector<ModeConfig> mode_configs;
        for (const auto &mode_yaml : player_yaml["modes"]) {
            ModeConfig mode_config = GetModeConfig(mode_yaml);
            mode_configs.push_back(mode_config);
        }
        ret = new CommonPlayer(player_name, page_configs, mode_configs);
    }
    return ret;
}

bool CommonApplication::QueryCurrentPlayerCallback(
    const std::string &request_str, std::string &response_str) {
    response_str = player_->Name();
    LOG_INFO("Get current player %s", response_str.c_str());
    return true;
}

bool CommonApplication::ReplaceCurrentPlayerCallback(
    const std::string &request_str, std::string &response_str) {
    LOG_INFO("Application set player %s", request_str.c_str());
    if (!SetPlayer(request_str)) {
        LOG_ERROR("Set player failed");
        return false;
    }
    return true;
}

bool CommonApplication::QueryCurrentModeCallback(const std::string &request_str,
                                                 std::string &response_str) {
    response_str = player_->GetMode();
    LOG_INFO("Get current mode %s", response_str.c_str());
    return true;
}

bool CommonApplication::ReplaceCurrentModeCallback(
    const std::string &request_str, std::string &response_str) {
    LOG_INFO("Application set mode %s", request_str.c_str());
    if (!player_->SetMode(request_str)) {
        LOG_ERROR("Set mode failed");
        return false;
    }
    return true;
}

bool CommonApplication::QueryStatusCallback(const std::string &request_str,
                                            std::string &response_str) {
    switch (status_) {
    case ApplicationStatus::Stopped:
        response_str = "stop";
        break;
    case ApplicationStatus::Pausing:
        response_str = "pause";
        break;
    case CommonApplication::Running:
        response_str = "running";
        break;
    case CommonApplication::Over:
        response_str = "over";
        break;
    default:
        LOG_ERROR("Status unkown status");
        response_str = "";
        break;
    }
    LOG_INFO("Get status %s", response_str.c_str());
    return !response_str.empty();
}

bool CommonApplication::ReplaceStatusCallback(const std::string &request_str,
                                              std::string &response_str) {
    LOG_INFO("Replace status %s", request_str.c_str());
    if (request_str == "running") {
        Continue();
    } else if (request_str == "pause") {
        Pause();
    } else if (request_str == "stop") {
        Stop();
    } else {
        LOG_ERROR("Unkown status changed. %s", request_str.c_str());
        return false;
    }
    return true;
}
