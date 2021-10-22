
#pragma once

#include "player.h"
#include <regex>
#include <mutex>

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
    PlayOperationType type;

    std::regex pattern;  // Click
    int sleep_time;  // Sleep
};

struct ModeConfig {
    std::string name;
    std::map<std::string, std::vector<ActionConfig>> page_to_actions;
};

// Common player
class CommonPlayer : public IPlayer {
public:
    CommonPlayer(const std::vector<PageConfig> &page_configs, const std::vector<ModeConfig> &mode_configs);

    ~CommonPlayer() override;

    bool Init() override;

    std::vector<PlayOperation>
    Play(const std::vector<ObjectBox> &object_boxes,
         const std::vector<TextBox> &text_boxes) override;

    bool IsGameOver() override;

    bool SetMode(const std::string &mode) override;

private:
    const std::vector<PageConfig> page_configs_;
    const std::vector<ModeConfig> mode_configs_;

    const ModeConfig *cur_mode_;
    std::mutex mutex_;
};
