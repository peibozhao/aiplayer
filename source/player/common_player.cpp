
#include "common_player.h"
#include "common/log.h"

static bool CenterInRange(int x, int y, std::pair<int, int> x_range,
                          std::pair<int, int> y_range) {
    return x > x_range.first && x < x_range.second && y > y_range.first &&
           y < y_range.second;
}

static bool SatisfyObject(const std::vector<ObjectBox> &obj_boxes,
                          const Condition &condition) {
    for (const ObjectBox &obj_box : obj_boxes) {
        if (std::regex_match(obj_box.name, condition.pattern) &&
            CenterInRange(obj_box.x, obj_box.y, condition.x_range,
                          condition.y_range)) {
            return true;
        }
    }
    return false;
}

#include <iostream>

static bool SatisfyText(const std::vector<TextBox> &text_boxes,
                        const Condition &condition) {
    for (const TextBox &ocr_box : text_boxes) {
        if (std::regex_match(ocr_box.text, condition.pattern) &&
            CenterInRange(ocr_box.x, ocr_box.y, condition.x_range,
                          condition.y_range)) {
            return true;
        }
    }
    return false;
}

static std::pair<int, int>
GetPatternPoint(const std::vector<ObjectBox> &obj_boxes,
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

CommonPlayer::CommonPlayer(const std::vector<PageConfig> &page_configs) {
    page_configs_ = page_configs;
}

CommonPlayer::~CommonPlayer() {}

bool CommonPlayer::Init() {
    for (int page_config_idx = 0; page_config_idx < page_configs_.size();
         ++page_config_idx) {
        const PageConfig &page_config = page_configs_[page_config_idx];

        if (page_config.name.empty())
            page_names_.push_back("page_" + std::to_string(page_config_idx));
        else
            page_names_.push_back(page_config.name);

        PageCondition page_condition;
        for (const ConditionConfig &condition_config :
             page_config.condition_configs) {
            Condition condition;
            condition.pattern.assign(condition_config.pattern);
            condition.x_range =
                std::make_pair(condition_config.x_min, condition_config.x_max);
            condition.y_range =
                std::make_pair(condition_config.y_min, condition_config.y_max);
            page_condition.push_back(condition);
        }
        page_conditions_.push_back(page_condition);

        PageAction page_action;
        for (const ActionConfig &action_config : page_config.action_configs) {
            Action action;
            if (action_config.type == "sleep") {
                action.type = PlayOperationType::SLEEP;
                action.sleep_time = action_config.sleep_time;
            } else if (action_config.type == "click") {
                action.type = PlayOperationType::SCREEN_CLICK;
                action.pattern.assign(action_config.pattern);
            } else {
                throw std::runtime_error("Unsupport action type " + action_config.type);
            }
            page_action.push_back(action);
        }
        page_actions_.push_back(page_action);
    }
    return true;
}

std::vector<PlayOperation>
CommonPlayer::Play(const std::vector<ObjectBox> &object_boxes,
                   const std::vector<TextBox> &text_boxes) {
    for (int page_idx = 0; page_idx < page_conditions_.size(); ++page_idx) {
        const PageCondition &page_condition = page_conditions_[page_idx];
        bool satified = true;
        for (const auto &condition : page_condition) {
            if (!SatisfyObject(object_boxes, condition) &&
                !SatisfyText(text_boxes, condition)) {
                satified = false;
                break;
            }
        }
        if (satified) {
            LOG_INFO("Detect page %s", page_names_[page_idx].c_str());
            std::vector<PlayOperation> ret;
            const PageAction &page_action = page_actions_[page_idx];
            for (const Action &action : page_action) {
                if (action.type == PlayOperationType::SCREEN_CLICK) {
                    auto point =
                        GetPatternPoint(object_boxes, text_boxes, action.pattern);
                    PlayOperation operation(PlayOperationType::SCREEN_CLICK);
                    operation.click.x = point.first;
                    operation.click.y = point.second;
                    ret.push_back(operation);
                } else if (action.type == PlayOperationType::SLEEP) {
                    PlayOperation operation(PlayOperationType::SLEEP);
                    operation.sleep_ms = action.sleep_time;
                    ret.push_back(operation);
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
