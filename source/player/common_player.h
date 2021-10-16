
#pragma once

#include "player.h"
#include <regex>

struct ConditionConfig {
    std::string pattern;
    int x_min, x_max;
    int y_min, y_max;
};

struct ActionConfig {
    std::string type;

    std::string pattern;
    int sleep_time;
};

struct Condition {
    std::regex pattern;
    std::pair<int, int> x_range;
    std::pair<int, int> y_range;
};

struct Action {
    PlayOperationType type;
    // TODO
    std::regex pattern;  // click
    int sleep_time;  // sleep
};

typedef std::vector<ConditionConfig> PageConditionConfig;
typedef std::vector<Condition> PageCondition;
typedef std::vector<ActionConfig> PageActionConfig;
typedef std::vector<Action> PageAction;

struct PageConfig {
    std::string name;
    PageConditionConfig condition_configs;
    PageActionConfig action_configs;
};

class CommonPlayer : public IPlayer {
public:
    CommonPlayer(const std::vector<PageConfig> &page_configs);

    ~CommonPlayer() override;

    bool Init() override;

    std::vector<PlayOperation>
    Play(const std::vector<ObjectBox> &object_boxes,
         const std::vector<TextBox> &text_boxes) override;

    bool IsGameOver() override;

private:
    std::vector<std::string> page_names_;
    std::vector<PageConfig> page_configs_;
    std::vector<PageCondition> page_conditions_;
    std::vector<PageAction> page_actions_;
};
