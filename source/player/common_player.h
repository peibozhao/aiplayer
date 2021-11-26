
#pragma once

#include "player.h"
#include <mutex>
#include <regex>
#include <atomic>

// Page
struct PageConditionConfig {
    std::regex pattern;
    int x_min, x_max;
    int y_min, y_max;
};

struct PageConfig {
    std::string name;
    std::vector<PageConditionConfig> condition_configs;
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
    std::vector<std::tuple<std::regex, std::vector<ActionConfig>>> page_pattern_actions;
    std::vector<ActionConfig> other_page_actions;
    std::vector<ActionConfig> undefined_page_actions;
};

// Common player
class CommonPlayer : public IPlayer {
public:
    CommonPlayer(const std::vector<PageConfig> &page_configs,
                 const std::vector<ModeConfig> &mode_configs, int width, int height);

    ~CommonPlayer() override;

    bool Init() override;

    std::vector<PlayOperation> Play(const std::vector<ObjectBox> &object_boxes,
                                    const std::vector<TextBox> &text_boxes) override;

    bool GameOver() override;

    void GameContinue() override;

    bool SetMode(const std::string &mode) override;

    std::string GetMode() override;

private:
    // Maybe change is_over_ flag
    std::vector<PlayOperation>
    CreatePlayOperation(const std::vector<ObjectBox> &object_boxes,
                        const std::vector<TextBox> &text_boxes,
                        const std::vector<ActionConfig> &action_configs);

private:
    int width_, height_;
    const std::vector<PageConfig> page_configs_;
    const std::vector<ModeConfig> mode_configs_;

    std::mutex mode_mutex_;
    const ModeConfig *cur_mode_;

    std::atomic<bool> is_over_;
};
