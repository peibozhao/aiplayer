
#pragma once

#include "player.h"
#include <regex>

struct ConditionConfig {
    std::string pattern;
    float x_min, x_max;  // [0, 1]
    float y_min, y_max;
};

struct Action {
    std::string name;
};

struct Condition {
    std::regex pattern;
    std::pair<float, float> x_range;  // [0, 1]
    std::pair<float, float> y_range;
};

typedef std::vector<ConditionConfig> PageConditionConfig;
typedef std::vector<Condition> PageCondition;
typedef std::vector<Action> PageAction;

class CommonPlayer : public IPlayer {
public:
    CommonPlayer(const std::vector<PageConditionConfig> &condition_configs,
                 const std::vector<PageAction> &ops);

    ~CommonPlayer() override;

    std::vector<PlayOperation>
    Play(const std::vector<ObjectBox> &object_boxes,
         const std::vector<TextBox> &text_boxes) override;

    bool IsGameOver() override;

private:
    std::vector<PageCondition> page_conditions_;
    std::vector<PageAction> page_ops_;
};
