
#pragma once

#include "blhx_player.h"
#include <map>
#include <regex>

///< 演习模式
class YanxiPlayer : public IBLHXPlayer {
public:
    bool Init(const std::string &cfg) override;

    bool Play(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts,
              std::vector<PlayOperation> &operations) override;

    bool GetLimit() override;

private:
  int width_, height_;
  int continuous_chuji_nums_;  // 连续识别出出击的次数
};

