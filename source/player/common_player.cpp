
#include "common_player.h"
#include "utils/image_utils.h"
#include "utils/log.h"
#include <cassert>

typedef std::pair<uint16_t, uint16_t> Point;
typedef std::array<float, 4> Region;

static bool CenterInRegion(const FrameSize &frame_size, const Region &region,
                           const Point &point) {
    return point.first > region[0] * frame_size.first &&
           point.first < region[2] * frame_size.first &&
           point.second > region[1] * frame_size.second &&
           point.second < region[3] * frame_size.second;
}

static bool
SatisfyPageCondition(const FrameSize frame_size,
                     const std::vector<ObjectBox> &obj_boxes,
                     const std::vector<TextBox> &text_boxes,
                     const std::vector<PageKeyElement> &key_elements) {
    for (const PageKeyElement &key_element : key_elements) {
        int box_idx = 0;
        for (; box_idx < obj_boxes.size() + text_boxes.size(); box_idx += 1) {
            std::string box_name;
            uint16_t x, y;
            if (box_idx < obj_boxes.size()) {
                box_name = obj_boxes[box_idx].name;
                x = obj_boxes[box_idx].x;
                y = obj_boxes[box_idx].y;
            } else {
                const TextBox &text_box =
                    text_boxes[box_idx - obj_boxes.size()];
                box_name = text_box.text;
                x = text_box.x;
                y = text_box.y;
            }

            if (std::regex_match(box_name, key_element.pattern) &&
                CenterInRegion(frame_size,
                               {key_element.x_min, key_element.y_min,
                                key_element.x_max, key_element.y_max},
                               {x, y})) {
                break;
            }
        }
        if (box_idx == obj_boxes.size() + text_boxes.size()) {
            return false;
        }
    }
    return true;
}

static std::tuple<std::string, int, int>
GetPatternPoint(const std::vector<ObjectBox> &obj_boxes,
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

CommonPlayer::CommonPlayer(const std::string &name,
                           const std::vector<PageConfig> &page_configs,
                           const std::vector<ModeConfig> &mode_configs)
    : IPlayer(name), page_configs_(page_configs), mode_configs_(mode_configs) {
    mode_ = nullptr;
    is_over_ = false;
}

CommonPlayer::~CommonPlayer() {}

bool CommonPlayer::Init() {
    if (mode_configs_.empty()) {
        LOG_ERROR("Mode is empty");
        return false;
    }
    mode_ = &mode_configs_.front();
    for (const ModeConfig &mode_config : mode_configs_) {
        for (const auto &pattern_action : mode_config.page_pattern_actions) {
            bool has_page = false;
            for (const PageConfig &page_config : page_configs_) {
                if (std::regex_match(page_config.name,
                                     std::get<0>(pattern_action))) {
                    has_page = true;
                    break;
                }
            }
            if (!has_page) {
                LOG_ERROR("Mode has unmatched page pattern. %s",
                          mode_config.name.c_str());
                return false;
            }
        }
    }
    return true;
}

std::vector<PlayOperation>
CommonPlayer::Play(const Image &image,
                   const std::vector<ObjectBox> &object_boxes,
                   const std::vector<TextBox> &text_boxes) {
    assert(image.format == ImageFormat::YUV420);

    FrameSize frame_size = ImageSize(image);
    std::lock_guard<std::mutex> lock(mode_mutex_);
    for (const PageConfig &page_config : page_configs_) {
        // Detect page
        if (!SatisfyPageCondition(frame_size, object_boxes, text_boxes,
                                  page_config.key_elements)) {
            continue;
        }
        LOG_INFO("Detect page %s", page_config.name.c_str());

        // Return page actions
        auto iter = mode_->page_pattern_actions.begin();
        for (; iter != mode_->page_pattern_actions.end(); ++iter) {
            if (std::regex_match(page_config.name, std::get<0>(*iter))) {
                break;
            }
        }
        std::vector<PlayOperation> ret;
        if (iter != mode_->page_pattern_actions.end()) {
            ret = CreatePlayOperation(frame_size.first, frame_size.second,
                                      object_boxes, text_boxes,
                                      std::get<1>(*iter));
        } else {
            LOG_INFO("Other page operation");
            ret = CreatePlayOperation(frame_size.first, frame_size.second,
                                      object_boxes, text_boxes,
                                      mode_->other_page_actions);
        }
        return ret;
    }

    // Page is not defined
    LOG_INFO("Undefined page operation");
    return CreatePlayOperation(frame_size.first, frame_size.second,
                               object_boxes, text_boxes,
                               mode_->undefined_page_actions);
}

bool CommonPlayer::GameOver() {
    if (is_over_) {
        LOG_INFO("Game over.");
    }
    return is_over_;
}

void CommonPlayer::GameContinue() {
    LOG_INFO("Game continue");
    is_over_ = false;
}

bool CommonPlayer::SetMode(const std::string &mode) {
    std::lock_guard<std::mutex> lock(mode_mutex_);
    for (const auto &mode_config : mode_configs_) {
        if (mode_config.name == mode) {
            mode_ = &mode_config;
            LOG_INFO("Player mode %s", mode_->name.c_str());
            is_over_ = false;
            return true;
        }
    }
    LOG_ERROR("Unsupport mode %s", mode.c_str());
    return false;
}

std::string CommonPlayer::GetMode() {
    std::lock_guard<std::mutex> lock(mode_mutex_);
    return mode_->name;
}

std::vector<PlayOperation> CommonPlayer::CreatePlayOperation(
    uint16_t width, uint16_t height, const std::vector<ObjectBox> &object_boxes,
    const std::vector<TextBox> &text_boxes,
    const std::vector<ActionConfig> &action_configs) {
    std::vector<PlayOperation> ret;
    for (const ActionConfig &action_config : action_configs) {
        if (action_config.type == "click") {
            PlayOperation operation(PlayOperationType::SCREEN_CLICK);
            if (action_config.pattern) {
                auto point_info = GetPatternPoint(object_boxes, text_boxes,
                                                  *action_config.pattern);
                std::string click_text = std::get<0>(point_info);
                if (click_text.empty()) {
                    LOG_ERROR("Can not find click pattern");
                    return {};
                }
                LOG_INFO("Click %s", std::get<0>(point_info).c_str());
                operation.click.x = std::get<1>(point_info);
                operation.click.y = std::get<2>(point_info);
            } else if (action_config.point) {
                operation.click.x = width * action_config.point->first;
                operation.click.y = height * action_config.point->second;
                LOG_INFO("Click point %d %d", operation.click.x,
                         operation.click.y);
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
