
#include "common_player.h"

static bool HasObject(const std::vector<ObjectBox> &obj_boxes,
                      const std::regex &pattern) {
    for (const ObjectBox &obj_box : obj_boxes) {
        if (std::regex_match(obj_box.name, pattern)) {
            return true;
        }
    }
    return false;
}

static bool HasText(const std::vector<TextBox> &text_boxes,
                    const std::regex &pattern) {
    for (const TextBox &ocr_box : text_boxes) {
        if (std::regex_match(ocr_box.text, pattern)) {
            return true;
        }
    }
    return false;
}

static std::pair<int, int> GetNamePoint(const std::vector<ObjectBox> &obj_boxes,
                                        const std::vector<TextBox> &text_boxes,
                                        const std::string &name) {
    for (const auto obj_box : obj_boxes) {
        if (obj_box.name == name) {
            return std::make_pair(obj_box.x + obj_box.width / 2,
                                  obj_box.y + obj_box.height / 2);
        }
    }
    for (const auto text_box : text_boxes) {
        if (text_box.text == name) {
            return std::make_pair(text_box.x + text_box.width / 2,
                                  text_box.y + text_box.height / 2);
        }
    }
    return std::make_pair(0, 0);
}

CommonPlayer::CommonPlayer(
    const std::vector<PageConditionConfig> &condition_configs,
    const std::vector<PageAction> &ops) {
    for (const PageConditionConfig &condition_config : condition_configs) {
        PageCondition page_condition;
        for (const ConditionConfig config : condition_config) {
            Condition condition;
            condition.pattern.assign(config.pattern);
            condition.x_range = std::make_pair(config.x_min, config.x_max);
            condition.y_range = std::make_pair(config.y_min, config.y_max);
            page_condition.push_back(condition);
        }
        page_conditions_.push_back(page_condition);
    }
    page_ops_ = ops;
}

CommonPlayer::~CommonPlayer() {}

std::vector<PlayOperation>
CommonPlayer::Play(const std::vector<ObjectBox> &object_boxes,
                   const std::vector<TextBox> &text_boxes) {
    for (int page_idx = 0; page_idx < page_conditions_.size(); ++page_idx) {
        const PageCondition &page_condition = page_conditions_[page_idx];
        bool is_meet = true;
        for (const auto &condition : page_condition) {
            if (!HasObject(object_boxes, condition.pattern) &&
                !HasText(text_boxes, condition.pattern)) {
                is_meet = false;
                break;
            }
        }
        if (is_meet) {
            std::vector<PlayOperation> ret;
            const PageAction &page_op = page_ops_[page_idx];
            for (const Action &op : page_op) {
                auto point = GetNamePoint(object_boxes, text_boxes, op.name);
                PlayOperation operation(PlayOperationType::SCREEN_CLICK);
                operation.click.x = point.first;
                operation.click.y = point.second;
                ret.push_back(operation);
            }
            return ret;
        }
    }
    return {};
}

bool CommonPlayer::IsGameOver() { return false; }
