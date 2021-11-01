
#include "blhx_application.h"
#include "common/log.h"
#include "ocr_detect/paddle_ocr.h"
#include "player/common_player.h"
#include "sink/dummy_operation.h"
#include "sink/minitouch_operation.h"
#include "source/image/image_source.h"
#include "source/image/minicap_source.h"
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

BlhxApplication::BlhxApplication(const std::string &config_fname) {
    status_ = ApplicationStatus::Stopped;
    config_fname_ = config_fname;
}

bool BlhxApplication::Init() {
    std::ifstream config_stream(config_fname_);
    YAML::Node config_yaml(YAML::Load(config_stream));

    // Image source
    const YAML::Node &source_yaml(config_yaml["source"]);
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
    const YAML::Node &ocr_yaml(config_yaml["ocr"]);
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
    const YAML::Node &player_yaml(config_yaml["player"]);
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
            for (auto &pipeline_yaml : mode_yaml["pipeline"]) {
                std::string page_name = pipeline_yaml["page"].as<std::string>();
                std::vector<ActionConfig> action_configs;
                for (auto &action_yaml : pipeline_yaml["actions"]) {
                    ActionConfig action_config;
                    std::string action_type = action_yaml["type"].as<std::string>();
                    action_config.type = action_type;
                    if (action_type == "click") {
                        action_config.pattern = action_yaml["pattern"].as<std::string>();
                    } else if (action_type == "click-pos") {
                        action_config.point.first = action_yaml["point"][0].as<float>();
                        action_config.point.second = action_yaml["point"][1].as<float>();
                    } else if (action_type == "sleep") {
                        action_config.sleep_time = action_yaml["time"].as<int>();
                    }
                    action_configs.push_back(action_config);
                }
                mode_config.page_to_actions[page_name] = action_configs;
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
    const YAML::Node &operation_yaml = config_yaml["operation"];
    std::string operation_type = operation_yaml["type"].as<std::string>();
    if (operation_type == "minitouch") {
        auto operation_server_info(GetServerInfo(operation_yaml));
        int orientation =
            source_yaml["orientation"].IsDefined() ? source_yaml["orientation"].as<int>() : 0;
        int delay_ms = operation_yaml["delay"].IsDefined() ? operation_yaml["delay"].as<int>() : 0;
        operation_.reset(new MinitouchOperation(std::get<0>(operation_server_info),
                                                std::get<1>(operation_server_info), image_info,
                                                orientation, delay_ms));
    } else if (operation_type == "dummy") {
        operation_.reset(new DummyOperation());
    }
    if (!operation_ || !operation_->Init()) {
        LOG_ERROR("operation init failed");
        return false;
    }
    LOG_INFO("Operation init success");

    return true;
}

void BlhxApplication::Run() {
    source_->Start();

    LOG_INFO("Application running");
    status_ = ApplicationStatus::Running;
    while (true) {
        std::unique_lock<std::mutex> lock(status_mutex_);
        if (status_ == ApplicationStatus::Stopped) {
            LOG_INFO("Application stoped");
            break;
        }
        if (player_->IsGameOver()) {
            LOG_INFO("Application is over");
            status_ = ApplicationStatus::Over;
        }
        status_con_.wait(lock, [this] { return status_ == ApplicationStatus::Running; });
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
        if (play_operations.empty()) {
            LOG_INFO("Operation is empty");
            continue;
        }

        TimeLog operation_time_log("Operation");
        for (const PlayOperation &play_operation : play_operations) {
            if (play_operation.type == PlayOperationType::SCREEN_CLICK) {
                operation_->Click(play_operation.click.x, play_operation.click.y);
            } else if (play_operation.type == PlayOperationType::SLEEP) {
                std::this_thread::sleep_for(std::chrono::milliseconds(play_operation.sleep_ms));
            }
        }
        operation_time_log.Tok();
    }
}

void BlhxApplication::Pause() {
    LOG_INFO("Application pause");
    source_->Stop();
    status_ = ApplicationStatus::Pausing;
}

void BlhxApplication::Continue() {
    LOG_INFO("Application continue");
    source_->Start();
    status_ = ApplicationStatus::Running;
    status_con_.notify_one();
}

void BlhxApplication::Stop() {
    LOG_INFO("Application stop");
    source_->Stop();
    status_ = ApplicationStatus::Stopped;
}

bool BlhxApplication::SetParam(const std::string &key, const std::string &value) {
    if (key == "mode") {
        LOG_INFO("Set param %s to %s", key.c_str(), value.c_str());
        if (player_->SetMode(value)) {
            std::lock_guard<std::mutex> lock(status_mutex_);
            status_ = ApplicationStatus::Running;
            return true;
        }
        return false;
    } else {
        LOG_ERROR("Unknown param. %s", key.c_str());
        return false;
    }
}
