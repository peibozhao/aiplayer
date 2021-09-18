
#include "blhx_application.h"
#include "device_operation/minitouch_operation.h"
#include "image_source/minicap_source.h"
#include "ocr_detect/paddle_ocr.h"
#include "player/common_player.h"
#include "yaml-cpp/yaml.h"
#include "common/util_defines.h"
#include <fstream>

static std::tuple<std::string, unsigned short> GetServerInfo(const YAML::Node &config_yaml) {
    return std::make_tuple(config_yaml["ip"].as<std::string>(),
                           config_yaml["port"].as<unsigned short>());
}

BlhxApplication::BlhxApplication(const std::string &config_fname) {
    running_ = false;
    config_fname_ = config_fname;
}

bool BlhxApplication::Init() {
    std::ifstream config_stream(config_fname_);
    YAML::Node config_yaml(YAML::Load(config_stream));

    const YAML::Node &source_yaml(config_yaml["source"]);
    auto source_server_info(GetServerInfo(source_yaml));
    source_.reset(
        new MinicapSource(std::get<0>(source_server_info), std::get<1>(source_server_info)));
    if (!source_->Init()) {
        LOG_ERROR("source init failed");
        return false;
    }

    const YAML::Node &ocr_yaml(config_yaml["ocr"]);
    auto ocr_server_info(GetServerInfo(ocr_yaml));
    ocr_.reset(new PaddleOcr(std::get<0>(ocr_server_info), std::get<1>(ocr_server_info)));
    if (!ocr_->Init()) {
        LOG_ERROR("ocr init failed");
        return false;
    }

    const YAML::Node &player_yaml(config_yaml["player"]);
    std::vector<PageConditionConfig> page_condition_configs;
    std::vector<PageAction> page_actions;
    for (auto page_yaml : player_yaml["pages"]) {
        PageConditionConfig page_condition_config;
        for (auto page_condition_yaml : page_yaml["conditions"]) {
            ConditionConfig condition_config;
            condition_config.pattern = page_condition_yaml["ocr_pattern"].as<std::string>();
            condition_config.x_min = page_condition_yaml["x_min"].as<unsigned short>();
            condition_config.x_max = page_condition_yaml["x_max"].as<unsigned short>();
            condition_config.y_min = page_condition_yaml["y_min"].as<unsigned short>();
            condition_config.y_max = page_condition_yaml["y_max"].as<unsigned short>();
            page_condition_config.push_back(condition_config);
        }
        PageAction page_action;
        for (auto page_action_yaml : page_yaml["actions"]) {
            Action action;
            action.name = page_action_yaml["name"].as<std::string>();
            page_action.push_back(action);
        }
    }
    player_.reset(new CommonPlayer(page_condition_configs, page_actions));
    if (!player_->Init()) {
        LOG_ERROR("player init failed");
        return false;
    }

    const YAML::Node &operation_yaml = config_yaml["operation"];
    auto operation_server_info(GetServerInfo(operation_yaml));
    operation_.reset(new MinitouchOperation(std::get<0>(operation_server_info),
                                            std::get<1>(operation_server_info)));
    if (!operation_->Init()) {
        LOG_ERROR("operation init failed");
        return false;
    }

    return true;
}

void BlhxApplication::Run() {
    running_ = true;
    while (running_) {
        ImageFormat image_format = source_->GetFormat();
        std::vector<char> image_buffer = source_->GetImageBuffer();

        std::vector<TextBox> text_boxes = ocr_->Detect(image_format, image_buffer);

        std::vector<PlayOperation> play_operations = player_->Play({}, text_boxes);

        for (const PlayOperation &play_operation : play_operations) {
            if (play_operation.type == PlayOperationType::SCREEN_CLICK) {
                int touch_id = operation_->TouchDown(play_operation.click.x, play_operation.click.y);
                operation_->TouchUp(touch_id);
            }
        }
    }
}

void BlhxApplication::Stop() {
    running_ = false;
}
