
#include "blhx_application.h"
#include <arpa/inet.h>
#include <fstream>
#include <netdb.h>
#include "yaml-cpp/yaml.h"
#include "common/log.h"
#include "source/minicap_source.h"
#include "source/image_source.h"
#include "ocr_detect/paddle_ocr.h"
#include "player/common_player.h"
#include "sink/minitouch_operation.h"
#include "sink/dummy_operation.h"

static std::tuple<std::string, unsigned short>
GetServerInfo(const YAML::Node &config_yaml) {
    std::string host_name = config_yaml["host"].as<std::string>();
    unsigned short port = config_yaml["port"].as<unsigned short>();
    struct hostent *host = gethostbyname(host_name.c_str());
    if (host == nullptr) {
        LOG_ERROR("Get ip failed %s", host_name.c_str());
        return std::make_tuple("", port);
    }
    return std::make_tuple(inet_ntoa(*(struct in_addr *)host->h_addr_list[0]),
                           port);
}

static PageConditionConfig GetConditionConfig(const YAML::Node &yaml_node,
                                          int width, int height) {
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
    int image_width = source_yaml["width"].as<int>(),
        image_height = source_yaml["height"].as<int>(),
        orientation = source_yaml["orientation"].IsDefined()
                          ? source_yaml["orientation"].as<int>()
                          : 0;
    std::string source_type = source_yaml["type"].as<std::string>();
    if (source_type == "minicap") {
        auto source_server_info(GetServerInfo(source_yaml));
        source_.reset(new MinicapSource(std::get<0>(source_server_info),
                                        std::get<1>(source_server_info)));
    } else if (source_type == "image") {
        source_.reset(new ImageSource(source_yaml["file_name"].as<std::string>()));
    }
    if (!source_ || !source_->Init()) {
        LOG_ERROR("source init failed");
        return false;
    }
    LOG_INFO("Image source init success");

    // OCR
    const YAML::Node &ocr_yaml(config_yaml["ocr"]);
    if (ocr_yaml["type"].as<std::string>() == "paddleocr") {
        std::string host_name = ocr_yaml["host"].as<std::string>();
        unsigned short port = ocr_yaml["port"].as<unsigned short>();
        ocr_.reset(new PaddleOcr(host_name, port));
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
                    GetConditionConfig(page_condition_yaml, image_width, image_height);
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
                    if (action_type == "click") {
                        action_config.type = PlayOperationType::SCREEN_CLICK;
                        action_config.pattern = action_yaml["pattern"].as<std::string>();
                    } else if (action_type == "sleep") {
                        action_config.type = PlayOperationType::SLEEP;
                        action_config.sleep_time = action_yaml["time"].as<int>();
                    } else {
                        LOG_ERROR("Unsupport action type: %s", action_type.c_str());
                        return false;
                    }
                    action_configs.push_back(action_config);
                }
                mode_config.page_to_actions[page_name] = action_configs;
            }
            mode_configs.push_back(mode_config);
        }
        player_.reset(new CommonPlayer(page_configs, mode_configs));
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
        ImageInfo image_info;
        image_info.width = image_width;
        image_info.height = image_height;
        image_info.orientation = orientation;
        operation_.reset(new MinitouchOperation(
            std::get<0>(operation_server_info),
            std::get<1>(operation_server_info), image_info));
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
    LOG_INFO("Application running");
    status_ = ApplicationStatus::Running;
    while (true) {
        if (status_ == ApplicationStatus::Pausing) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        } else if (status_ == ApplicationStatus::Stopped) {
            break;
        }

        ImageFormat image_format = source_->GetFormat();
        TimeLog image_time_log("Image source");
        std::vector<char> image_buffer = source_->GetImageBuffer();
        if (image_buffer.empty()) {
            LOG_INFO("Image is empty");
            continue;
        }
        image_time_log.Tok();

        TimeLog ocr_time_log("OCR");
        std::vector<TextBox> text_boxes =
            ocr_->Detect(image_format, image_buffer);
        if (text_boxes.empty()) {
            LOG_INFO("Text is empty");
            continue;
        }
        ocr_time_log.Tok();

        std::vector<PlayOperation> play_operations =
            player_->Play({}, text_boxes);
        if (play_operations.empty()) {
            LOG_INFO("Operation is empty");
            continue;
        }

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
    }
}

void BlhxApplication::Pause() {
    LOG_INFO("Application pause");
    status_ = ApplicationStatus::Pausing;
}

void BlhxApplication::Continue() {
    LOG_INFO("Application continue");
    status_ = ApplicationStatus::Running;
}
