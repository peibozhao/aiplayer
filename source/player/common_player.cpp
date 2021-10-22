
#include "common_player.h"
#include "common/log.h"

static bool CenterInRange(int x, int y, int x_min, int x_max, int y_min, int y_max) {
    return x > x_min && x < x_max && y > y_min && y < y_max;
}

static bool SatisfyObject(const std::vector<ObjectBox> &obj_boxes,
                          const PageConditionConfig &condition) {
    for (const ObjectBox &obj_box : obj_boxes) {
        if (std::regex_match(obj_box.name, condition.pattern) &&
            CenterInRange(obj_box.x, obj_box.y, condition.x_min, condition.x_max, condition.y_min,
                          condition.y_max)) {
            return true;
        }
    }
    return false;
}

static bool SatisfyText(const std::vector<TextBox> &text_boxes,
                        const PageConditionConfig &condition) {
    for (const TextBox &ocr_box : text_boxes) {
        if (std::regex_match(ocr_box.text, condition.pattern) &&
            CenterInRange(ocr_box.x, ocr_box.y, condition.x_min, condition.x_max, condition.y_min,
                          condition.y_max)) {
            return true;
        }
    }
    return false;
}

static std::pair<int, int> GetPatternPoint(const std::vector<ObjectBox> &obj_boxes,
                                           const std::vector<TextBox> &text_boxes,
                                           const std::regex &pattern) {
    for (const auto obj_box : obj_boxes) {
        if (std::regex_match(obj_box.name, pattern)) {
            return std::make_pair(obj_box.x, obj_box.y);
        }
    }
    for (const auto text_box : text_boxes) {
        if (std::regex_match(text_box.text, pattern)) {
            return std::make_pair(text_box.x, text_box.y);
        }
    }
    return std::make_pair(0, 0);
}

CommonPlayer::CommonPlayer(const std::vector<PageConfig> &page_configs,
                           const std::vector<ModeConfig> &mode_configs)
    : page_configs_(page_configs), mode_configs_(mode_configs) {
    cur_mode_ = nullptr;
}

CommonPlayer::~CommonPlayer() {}

bool CommonPlayer::Init() {
    if (mode_configs_.empty()) {
        LOG_ERROR("Mode is empty");
        return false;
    }
    cur_mode_ = &mode_configs_.front();
    for (const ModeConfig &mode_config : mode_configs_) {
        for (const auto &page_to_actions : mode_config.page_to_actions) {
            bool has_page = false;
            for (const PageConfig &page_config : page_configs_) {
                if (page_config.name == page_to_actions.first) {
                    has_page = true;
                    break;
                }
            }
            if (!has_page) {
                LOG_ERROR("No such page: %s", page_to_actions.first.c_str());
                return false;
            }
        }
    }
    return true;
}

std::vector<PlayOperation> CommonPlayer::Play(const std::vector<ObjectBox> &object_boxes,
                                              const std::vector<TextBox> &text_boxes) {
    for (int page_idx = 0; page_idx < page_configs_.size(); ++page_idx) {
        const PageConfig &page_config = page_configs_[page_idx];
        std::string page_name;
        for (const auto &condition : page_config.condition_configs) {
            if (!SatisfyObject(object_boxes, condition) && !SatisfyText(text_boxes, condition)) {
                page_name = page_config.name;
                break;
            }
        }
        if (!page_name.empty()) {
            std::lock_guard<std::mutex> lock(mutex_);
            LOG_INFO("Detect page %s", page_name.c_str());
            auto iter = cur_mode_->page_to_actions.find(page_name);

            std::vector<PlayOperation> ret;
            if (iter != cur_mode_->page_to_actions.end()) {
                for (const ActionConfig &action_config : iter->second) {
                    if (action_config.type == PlayOperationType::SCREEN_CLICK) {
                        auto point =
                            GetPatternPoint(object_boxes, text_boxes, action_config.pattern);
                        PlayOperation operation(PlayOperationType::SCREEN_CLICK);
                        operation.click.x = point.first;
                        operation.click.y = point.second;
                        ret.push_back(operation);
                    } else if (action_config.type == PlayOperationType::SLEEP) {
                        PlayOperation operation(PlayOperationType::SLEEP);
                        operation.sleep_ms = action_config.sleep_time;
                        ret.push_back(operation);
                    }
                }
            }
            return ret;
        }
    }
    LOG_INFO("Undefined page");
    PlayOperation operation(PlayOperationType::SLEEP);
    operation.sleep_ms = 1000;
    return {operation};
}

bool CommonPlayer::IsGameOver() { return false; }

bool CommonPlayer::SetMode(const std::string &mode) {
    for (const auto &mode_config : mode_configs_) {
        if (mode_config.name == mode) {
            std::lock_guard<std::mutex> lock(mutex_);
            cur_mode_ = &mode_config;
            return true;
        }
    }
    LOG_ERROR("Unsupport mode %s", mode.c_str());
    return false;
}
