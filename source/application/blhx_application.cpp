
#include "blhx_application.h"
#include "common/log.h"
#include "common/util_functions.h"
#include "nlohmann/json.hpp"
#include "ocr_detect/paddle_ocr.h"
#include "player/common_player.h"
#include "sink/notify//miao_notify.h"
#include "sink/operation/dummy_operation.h"
#include "sink/operation/minitouch_operation.h"
#include "source/image/image_source.h"
#include "source/image/minicap_source.h"
#include "source/request/http_request.h"
#include "yaml-cpp/yaml.h"
#include <arpa/inet.h>
#include <fstream>
#include <netdb.h>

static std::tuple<std::string, unsigned short> GetServerInfo(const YAML::Node &config_yaml) {
    std::string host_name = config_yaml["host"].as<std::string>();
    unsigned short port = config_yaml["port"].as<unsigned short>();
    struct hostent *host = gethostbyname(host_name.c_str());
    if (host == nullptr) {
        LOG_ERROR("Get ip failed %s", host_name.c_str());
        return std::make_tuple("", port);
    }
    return std::make_tuple(inet_ntoa(*(struct in_addr *)host->h_addr_list[0]), port);
}

static PageConditionConfig GetConditionConfig(const YAML::Node &yaml_node, int width, int height) {
    PageConditionConfig ret;
    ret.pattern = yaml_node["pattern"].as<std::string>();
    if (yaml_node["x_range"].IsDefined()) {
        ret.x_min = yaml_node["x_range"][0].as<float>() * width;
        ret.x_max = yaml_node["x_range"][1].as<float>() * width;
    } else {
        ret.x_min = 0;
        ret.x_max = width;
    }
    if (yaml_node["y_range"].IsDefined()) {
        ret.y_min = yaml_node["y_range"][0].as<float>() * height;
        ret.y_max = yaml_node["y_range"][1].as<float>() * height;
    } else {
        ret.y_min = 0;
        ret.y_max = height;
    }
    return ret;
}

static ActionConfig CreateActionConfig(const YAML::Node &action_yaml) {
    ActionConfig action_config;
    std::string action_type = action_yaml["type"].as<std::string>();
    action_config.type = action_type;
    if (action_type == "click") {
        if (action_yaml["pattern"].IsDefined()) {
            action_config.pattern = action_yaml["pattern"].as<std::string>();
        } else if (action_yaml["point"].IsDefined()) {
            action_config.point = std::make_pair(action_yaml["point"][0].as<float>(),
                                                 action_yaml["point"][1].as<float>());
        }
    } else if (action_type == "sleep") {
        action_config.sleep_time = action_yaml["time"].as<int>();
    }
    return action_config;
}

BlhxApplication::BlhxApplication(const std::string &config_fname) {
    status_ = ApplicationStatus::Stopped;
    config_fname_ = config_fname;
}

bool BlhxApplication::Init() {
    std::ifstream config_stream(config_fname_);
    YAML::Node config_yaml(YAML::Load(config_stream));
    return InitByYaml(config_yaml);
}

void BlhxApplication::Run() {
    source_->Start();
    if (request_) { request_->Start(); }

    LOG_INFO("Application running");
    status_ = ApplicationStatus::Running;

    while (true) {
        std::unique_lock<std::mutex> lock(status_mutex_);
        status_con_.wait(lock, [this] {
            return status_ == ApplicationStatus::Running || status_ == ApplicationStatus::Stopped;
        });
        if (status_ == ApplicationStatus::Stopped) {
            LOG_INFO("Application stoped");
            break;
        }
        lock.unlock();

        ImageInfo image_info = source_->GetImageInfo();
        TimeLog image_time_log("Image source");
        std::vector<char> image_buffer = source_->GetImageBuffer();
        if (image_buffer.empty()) {
            LOG_INFO("Image is empty");
            continue;
        }
        image_time_log.Tok();

        TimeLog ocr_time_log("OCR");
        std::vector<TextBox> text_boxes = ocr_->Detect(image_info, image_buffer);
        if (text_boxes.empty()) {
            LOG_INFO("Text is empty");
            continue;
        }
        ocr_time_log.Tok();

        std::vector<PlayOperation> play_operations = player_->Play({}, text_boxes);
        if (!play_operations.empty()) {
            TimeLog operation_time_log("Operation");
            for (const PlayOperation &play_operation : play_operations) {
                if (play_operation.type == PlayOperationType::SCREEN_CLICK) {
                    operation_->Click(play_operation.click.x, play_operation.click.y);
                } else if (play_operation.type == PlayOperationType::SLEEP) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(play_operation.sleep_ms));
                }
            }
            operation_time_log.Tok();

            if (interval_ms_ > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
            }
        } else {
            LOG_INFO("Operation is empty");
        }

        if (player_->IsGameOver()) {
            LOG_INFO("Application is over");
            status_ = ApplicationStatus::Over;
            if (notify_) {
                LOG_INFO("Notify %s over event", player_->GetMode().c_str());
                if (!notify_->Notify("Mode " + player_->GetMode() + " is over")) {
                    LOG_ERROR("Notify failed.");
                }
            }
        }
    }
}

void BlhxApplication::Pause() {
    LOG_INFO("Application pause");
    std::lock_guard<std::mutex> lock(status_mutex_);
    status_ = ApplicationStatus::Pausing;
}

void BlhxApplication::Continue() {
    LOG_INFO("Application continue");
    std::lock_guard<std::mutex> lock(status_mutex_);
    status_ = ApplicationStatus::Running;
    status_con_.notify_one();
}

void BlhxApplication::Stop() {
    LOG_INFO("Application stop");
    source_->Stop();
    std::lock_guard<std::mutex> lock(status_mutex_);
    status_ = ApplicationStatus::Stopped;
    status_con_.notify_one();
}

bool BlhxApplication::SetParam(const std::string &key, const std::string &value) {
    LOG_INFO("Application set param %s to %s", key.c_str(), value.c_str());
    if (key == "mode") {
        LOG_INFO("Set param %s to %s", key.c_str(), value.c_str());
        if (!player_->SetMode(value)) {
            LOG_INFO("Set mode failed");
            return false;
        }
        std::lock_guard<std::mutex> lock(status_mutex_);
        status_ = ApplicationStatus::Running;
        status_con_.notify_one();
        return true;
    } else if (key == "status") {
        if (value == "running") {
            Continue();
        } else if (value == "pause") {
            Pause();
        } else if (value == "stop") {
            Stop();
        } else {
            LOG_ERROR("Unkown status changed. %s", key.c_str());
            return false;
        }
        return true;
    } else {
        LOG_ERROR("Unknown param. %s", key.c_str());
        return false;
    }
}

std::string BlhxApplication::GetParam(const std::string &key) {
    if (key == "status") {
        switch (status_) {
        case ApplicationStatus::Stopped:
            return "stop";
        case ApplicationStatus::Pausing:
            return "pause";
        case BlhxApplication::Running:
            return "running";
        case BlhxApplication::Over:
            return "over";
        default:
            LOG_ERROR("Status unkown status");
            return "";
        }
    } else if (key == "mode") {
        return player_->GetMode();
    } else {
        LOG_ERROR("Unknown param. %s", key.c_str());
        return "";
    }
}

bool BlhxApplication::InitByYaml(const YAML::Node &yaml) {
    // Image source
    const YAML::Node &source_yaml(yaml["source"]);
    std::string source_type = source_yaml["type"].as<std::string>();
    if (source_type == "minicap") {
        auto source_server_info(GetServerInfo(source_yaml));
        source_.reset(
            new MinicapSource(std::get<0>(source_server_info), std::get<1>(source_server_info)));
    } else if (source_type == "image") {
        ImageInfo image_info;
        image_info.format = ImageFormat::JPEG;
        image_info.width = source_yaml["width"].as<int>();
        image_info.height = source_yaml["height"].as<int>();
        source_.reset(new ImageSource(source_yaml["file_name"].as<std::string>(), image_info));
    }
    if (!source_ || !source_->Init()) {
        LOG_ERROR("source init failed");
        return false;
    }
    LOG_INFO("Image source init success");
    ImageInfo image_info = source_->GetImageInfo();

    // OCR
    const YAML::Node &ocr_yaml(yaml["ocr"]);
    if (ocr_yaml["type"].as<std::string>() == "paddleocr") {
        std::string host_name = ocr_yaml["host"].as<std::string>();
        unsigned short port = ocr_yaml["port"].as<unsigned short>();
        if (ocr_yaml["recv_timeout"].IsDefined()) {
            ocr_.reset(new PaddleOcr(host_name, port, ocr_yaml["recv_timeout"].as<int>()));
        } else {
            ocr_.reset(new PaddleOcr(host_name, port));
        }
    }
    if (!ocr_ || !ocr_->Init()) {
        LOG_ERROR("ocr init failed");
        return false;
    }
    LOG_INFO("OCR init success");

    // Player
    const YAML::Node &player_yaml(yaml["player"]);
    if (player_yaml["type"].as<std::string>() == "common") {
        std::vector<PageConfig> page_configs;
        for (auto &page_yaml : player_yaml["pages"]) {
            PageConfig page_config;
            page_config.name = page_yaml["name"].as<std::string>();

            for (auto &page_condition_yaml : page_yaml["conditions"]) {
                PageConditionConfig page_condition_config =
                    GetConditionConfig(page_condition_yaml, image_info.width, image_info.height);
                page_config.condition_configs.push_back(page_condition_config);
            }
            page_configs.push_back(page_config);
        }

        std::vector<ModeConfig> mode_configs;
        for (auto &mode_yaml : player_yaml["modes"]) {
            ModeConfig mode_config;
            mode_config.name = mode_yaml["name"].as<std::string>();
            for (auto &defined_page_yaml : mode_yaml["page_actions"]) {
                std::string page_pattern = defined_page_yaml["page"].as<std::string>();
                std::vector<ActionConfig> action_configs;
                for (auto &action_yaml : defined_page_yaml["actions"]) {
                    ActionConfig action_config = CreateActionConfig(action_yaml);
                    action_configs.push_back(action_config);
                }
                mode_config.page_pattern_actions.push_back(
                    std::make_tuple(std::regex(page_pattern), action_configs));
            }

            if (mode_yaml["other_page_actions"].IsDefined()) {
                std::vector<ActionConfig> action_configs;
                const YAML::Node &other_page_yaml = mode_yaml["other_page_actions"];
                for (auto action_yaml : other_page_yaml["actions"]) {
                    ActionConfig action_config = CreateActionConfig(action_yaml);
                    mode_config.other_page_actions.push_back(action_config);
                }
            }

            if (mode_yaml["undefined_page_actions"].IsDefined()) {
                std::vector<ActionConfig> action_configs;
                const YAML::Node &undefined_page_yaml = mode_yaml["undefined_page_actions"];
                for (auto action_yaml : undefined_page_yaml["actions"]) {
                    ActionConfig action_config = CreateActionConfig(action_yaml);
                    mode_config.undefined_page_actions.push_back(action_config);
                }
            }

            mode_configs.push_back(mode_config);
        }
        player_.reset(
            new CommonPlayer(page_configs, mode_configs, image_info.width, image_info.height));
    }
    if (!player_ || !player_->Init()) {
        LOG_ERROR("player init failed");
        return false;
    }
    LOG_INFO("Player init success");

    // Operation
    const YAML::Node &operation_yaml = yaml["operation"];
    std::string operation_type = operation_yaml["type"].as<std::string>();
    if (operation_type == "minitouch") {
        auto operation_server_info(GetServerInfo(operation_yaml));
        int orientation =
            source_yaml["orientation"].IsDefined() ? source_yaml["orientation"].as<int>() : 0;
        operation_.reset(new MinitouchOperation(std::get<0>(operation_server_info),
                                                std::get<1>(operation_server_info), image_info,
                                                orientation));
    } else if (operation_type == "dummy") {
        operation_.reset(new DummyOperation());
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
            auto request_server_info(GetServerInfo(request_yaml));
            request_.reset(new HttpRequest(std::get<0>(request_server_info),
                                           std::get<1>(request_server_info)));
        }
        if (!request_ || !request_->Init()) {
            request_.reset();
            LOG_ERROR("Request init failed");
        } else {
            request_->SetCallback("/current_mode/name", RequestOperation::Query,
                                  std::bind(&BlhxApplication::QueryCurrentModeCallback, this,
                                            std::placeholders::_1, std::placeholders::_2));

            request_->SetCallback("/current_mode/name", RequestOperation::Replace,
                                  std::bind(&BlhxApplication::ReplaceCurrentModeCallback, this,
                                            std::placeholders::_1, std::placeholders::_2));

            request_->SetCallback("/status", RequestOperation::Query,
                                  std::bind(&BlhxApplication::QueryStatusCallback, this,
                                            std::placeholders::_1, std::placeholders::_2));

            request_->SetCallback("/status", RequestOperation::Replace,
                                  std::bind(&BlhxApplication::ReplaceStatusCallback, this,
                                            std::placeholders::_1, std::placeholders::_2));

            request_->SetCallback("/image", RequestOperation::Query,
                                  std::bind(&BlhxApplication::QueryImage, this,
                                            std::placeholders::_1, std::placeholders::_2));

            request_->SetCallback("/operation", RequestOperation::Replace,
                                  std::bind(&BlhxApplication::ReplaceOperation, this,
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
            notify_.reset(new MiaoNotify(notify_yaml["id"].as<std::string>()));
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
        interval_ms_ = app_yaml["interval"].IsDefined() ? app_yaml["interval"].as<int>() : 0;
    }

    return true;
}

bool BlhxApplication::QueryCurrentModeCallback(const std::string &request_str,
                                               std::string &response_str) {
    response_str = GetParam("mode");
    LOG_INFO("Get current mode %s", response_str.c_str());
    return true;
}

bool BlhxApplication::ReplaceCurrentModeCallback(const std::string &request_str,
                                                 std::string &response_str) {
    LOG_INFO("Application set mode %s", request_str.c_str());
    bool ret = SetParam("mode", request_str);
    if (!ret) { LOG_ERROR("Player set mode failed"); }
    return ret;
}

bool BlhxApplication::QueryStatusCallback(const std::string &request_str,
                                          std::string &response_str) {
    response_str = GetParam("status");
    LOG_INFO("Get status %s", response_str.c_str());
    return !response_str.empty();
}

bool BlhxApplication::ReplaceStatusCallback(const std::string &request_str,
                                            std::string &response_str) {
    LOG_INFO("Replace status %s", request_str.c_str());
    return SetParam("status", request_str);
}

bool BlhxApplication::QueryImage(const std::string &request_str, std::string &response_str) {
    LOG_INFO("Get image");
    std::vector<char> img_buf = source_->GetImageBuffer();
    if (img_buf.empty()) {
        LOG_ERROR("Image is empty");
        return false;
    }
    std::vector<uint8_t> tmp(img_buf.size());
    std::transform(img_buf.begin(), img_buf.end(), tmp.begin(),
                   [](char c) { return static_cast<uint8_t>(c); });
    std::string img_base64 = Base64Encode(tmp);
    response_str = img_base64;
    return true;
}

bool BlhxApplication::ReplaceOperation(const std::string &request_str, std::string &response_str) {
    LOG_INFO("Request operation %s", request_str.c_str());
    auto split_iter = request_str.find(',');
    if (split_iter == request_str.npos) {
        LOG_ERROR("Param format error");
        return false;
    }
    int x = std::stoi(request_str.substr(0, split_iter));
    int y = std::stoi(request_str.substr(split_iter + 1));
    operation_->Click(x, y);
    return true;
}
