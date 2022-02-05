#pragma once

#include "player.h"
#include <atomic>
#include <mutex>
#include <regex>

/// @brief 页面的关键元素
struct PageKeyElement {
    std::regex pattern;
    float x_min, x_max;
    float y_min, y_max;
};

struct PageConfig {
    std::string name;
    std::vector<PageKeyElement> key_elements;
};

// Play mode
struct ActionConfig {
    std::string type;

    std::optional<std::regex> pattern;            // Click
    std::optional<std::pair<float, float>> point; // Click point
    std::optional<int> sleep_time;                // Sleep
};

struct ModeConfig {
    std::string name;
    std::vector<std::tuple<std::regex, std::vector<ActionConfig>>>
        page_pattern_actions;
    std::vector<ActionConfig>
        other_page_actions; // page已定义, 但是当前mode没有定义对应的action
    std::vector<ActionConfig> undefined_page_actions; // 没有定义的page的行为
};

/// @brief 给每个页面定义关键元素和操作, 如果检测到某个页面的全部关键元素,
/// 就认定屏幕当前为该页面, 并执行页面对应的操作
class CommonPlayer : public IPlayer {
public:
    CommonPlayer(const std::string &name,
                 const std::vector<PageConfig> &page_configs,
                 const std::vector<ModeConfig> &mode_configs);

    ~CommonPlayer() override;

    bool Init() override;

    std::vector<PlayOperation>
    Play(const Image &image, const std::vector<ObjectBox> &object_boxes,
         const std::vector<TextBox> &text_boxes) override;

    bool GameOver() override;

    void GameContinue() override;

    bool SetMode(const std::string &mode) override;

    std::string GetMode() override;

private:
    // Maybe change is_over_ flag
    std::vector<PlayOperation>
    CreatePlayOperation(uint16_t width, uint16_t height,
                        const std::vector<ObjectBox> &object_boxes,
                        const std::vector<TextBox> &text_boxes,
                        const std::vector<ActionConfig> &action_configs);

private:
    const std::vector<PageConfig> page_configs_;
    const std::vector<ModeConfig> mode_configs_;

    std::mutex mode_mutex_;
    const ModeConfig *mode_;

    std::atomic<bool> is_over_;
};
