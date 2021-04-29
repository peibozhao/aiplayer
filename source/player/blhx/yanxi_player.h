
#pragma once

#include "player/blhx_player.h"
#include <map>
#include <regex>

///< 演习模式
class YanxiPlayer : public IBLHXPlayer {
public:
    bool Init(std::istream &is) override;

    std::vector<PlayOperation> Play(const std::vector<ObjectBox> &object_boxes, const std::vector<TextBox> &text_boxes) override;

    bool GetLimit() override;

private:
  int width_, height_;
  int continuous_chuji_nums_;  // 连续识别出出击的次数
};

