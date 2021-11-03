
#include "common_player.h"
#include "common/log.h"

static bool CenterInRange(int x, int y, int x_min, int x_max, int y_min, int y_max) {
    return x > x_min && x < x_max && y > y_min && y < y_max;
}

static bool SatisfyPageCondition(const std::vector<ObjectBox> &obj_boxes,
                                 const std::vector<TextBox> &text_boxes,
                                 const std::vector<PageConditionConfig> &conditions) {
    if (conditions.empty()) { return true; }

    bool satisfy = false;
    for (const PageConditionConfig &condition : conditions) {
        satisfy = false;
        for (const ObjectBox &obj_box : obj_boxes) {
            satisfy = std::regex_match(obj_box.name, condition.pattern) &&
                      CenterInRange(obj_box.x, obj_box.y, condition.x_min, condition.x_max,
                                    condition.y_min, condition.y_max);
            if (satisfy) { break; }
        }

        if (!satisfy) {
            for (const TextBox &ocr_box : text_boxes) {
                satisfy = std::regex_match(ocr_box.text, condition.pattern) &&
                          CenterInRange(ocr_box.x, ocr_box.y, condition.x_min, condition.x_max,
                                        condition.y_min, condition.y_max);
                if (satisfy) { break; }
            }
        }

        if (!satisfy) { return false; }
    }
    return true;
}

static std::tuple<std::string, int, int> GetPatternPoint(const std::vector<ObjectBox> &obj_boxes,
                                                         const std::vector<TextBox> &text_boxes,
                                                         const std::regex &pattern) {
    for (const auto obj_box : obj_boxes) {
        if (std::regex_match(obj_box.name, pattern)) {
            return std::make_tuple(obj_box.name, obj_box.x, obj_box.y);
        }
    }
    for (const auto text_box : text_boxes) {
        if (std::regex_match(text_box.text, pattern)) {
            return std::make_tuple(text_box.text, text_box.x, text_box.y);
        }
    }
    return std::make_tuple("", 0, 0);
}

CommonPlayer::CommonPlayer(const std::vector<PageConfig> &page_configs,
                           const std::vector<ModeConfig> &mode_configs, int width, int height)
    : page_configs_(page_configs), mode_configs_(mode_configs) {
    cur_mode_ = nullptr;
    is_over_ = false;
    width_ = width;
    height_ = height;
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
    std::lock_guard<std::mutex> lock(mode_mutex_);
    for (const PageConfig &page_config : page_configs_) {
        // Detect page
        if (!SatisfyPageCondition(object_boxes, text_boxes, page_config.condition_configs)) {
            continue;
        }
        std::string page_name = page_config.name;
        LOG_INFO("Detect page %s", page_name.c_str());

        // Return page actions
        auto iter = cur_mode_->page_to_actions.find(page_name);

        std::vector<PlayOperation> ret;
        if (iter != cur_mode_->page_to_actions.end()) {
            ret = CreatePlayOperation(object_boxes, text_boxes, iter->second);
        } else {
            ret = CreatePlayOperation(object_boxes, text_boxes, cur_mode_->other_page_actions);
        }
        return ret;
    }

    // Page is not defined
    LOG_INFO("Undefined page");
    return CreatePlayOperation(object_boxes, text_boxes, cur_mode_->undefined_page_actions);
}

bool CommonPlayer::IsGameOver() {
    if (is_over_) { LOG_INFO("Mode play end."); }
    return is_over_;
}

bool CommonPlayer::SetMode(const std::string &mode) {
    std::lock_guard<std::mutex> lock(mode_mutex_);
    for (const auto &mode_config : mode_configs_) {
        if (mode_config.name == mode) {
            cur_mode_ = &mode_config;
            LOG_INFO("Player mode %s", cur_mode_->name.c_str());
            is_over_ = false;
            return true;
        }
    }
    LOG_ERROR("Unsupport mode %s", mode.c_str());
    return false;
}

std::string CommonPlayer::GetMode() {
    std::lock_guard<std::mutex> lock(mode_mutex_);
    return cur_mode_->name;
}

std::vector<PlayOperation>
CommonPlayer::CreatePlayOperation(const std::vector<ObjectBox> &object_boxes,
                                  const std::vector<TextBox> &text_boxes,
                                  const std::vector<ActionConfig> &action_configs) {
    std::vector<PlayOperation> ret;
    for (const ActionConfig &action_config : action_configs) {
        if (action_config.type == "click") {
            PlayOperation operation(PlayOperationType::SCREEN_CLICK);
            if (action_config.pattern) {
                auto point_info = GetPatternPoint(object_boxes, text_boxes, *action_config.pattern);
                std::string click_text = std::get<0>(point_info);
                if (click_text.empty()) {
                    LOG_ERROR("Can not find click pattern");
                    return {};
                }
                LOG_INFO("Click %s", std::get<0>(point_info).c_str());
                operation.click.x = std::get<1>(point_info);
                operation.click.y = std::get<2>(point_info);
            } else if (action_config.point) {
                operation.click.x = width_ * action_config.point->first;
                operation.click.y = height_ * action_config.point->second;
                LOG_INFO("Click point %d %d", operation.click.x, operation.click.y);
            }
            ret.push_back(operation);
        } else if (action_config.type == "sleep") {
            LOG_INFO("Sleep %d", *action_config.sleep_time);
            PlayOperation operation(PlayOperationType::SLEEP);
            operation.sleep_ms = *action_config.sleep_time;
            ret.push_back(operation);
        } else if (action_config.type == "over") {
            LOG_INFO("Play end");
            is_over_ = true;
        }
    }
    return ret;
}
