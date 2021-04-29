
#pragma once

#include "player/blhx_player.h"
#include <regex>

class BattlePlayer : public IBLHXPlayer {
public:
    BattlePlayer() {
        boss_appeared_ = false;
        width_ = 0;
        height_ = 0;
        boundary_width_ = 0;
        left_times_ = 0;
    }

    ~BattlePlayer() override {}

    bool Init(std::istream &is) override;

    std::vector<PlayOperation> Play(const std::vector<ObjectBox> &object_boxes,
                                    const std::vector<TextBox> &text_boxes) override;

    bool GetLimit() override;

private:
    std::vector<PlayOperation> AttackEnemy(const std::vector<ObjectBox> &object_boxes);

    std::vector<PlayOperation> Move(const ObjectBox &box);

    PlayOperation CheckBoundray(const PlayOperation &opt);

    void Reset();

    bool Match(const std::string &patter, const std::string &str);

private:
    bool boss_appeared_;
    int width_, height_;
    int boundary_width_;
    std::string chapter_pattern_, name_pattern_;
    int left_times_;
};
