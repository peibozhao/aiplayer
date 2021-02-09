
#pragma once

#include "blhx_player.h"
#include <regex>

///< 刷图模式
class BattlePlayer : public IBLHXPlayer {
public:
    bool Init(const std::string &cfg) override;

    bool Play(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts,
              std::vector<PlayOperation> &operations) override;

    bool GetLimit() override;

private:
    bool boss_appeared_;
    int width_, height_;
    int boundary_width_;
    std::regex chapter_reg_, name_reg_;
    int left_times_;
};
