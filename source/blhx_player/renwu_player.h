
#pragma once

#include "blhx_player.h"
#include <map>
#include <vector>

class RenwuPlayer : public IBLHXPlayer {
private:
  enum RenwuType {
    SCHW,
    HYTJ,
    ZSYX,
    ZSXD,
    PJZH,
    TOTAL_NUMS,
    NONE
  };

public:
    bool Init(const std::string &cfg) override;

    bool Play(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts,
              std::vector<PlayOperation> &operations) override;

    bool GetLimit() override;

private:
  int width_, height_;
  std::vector<int> renwu_left_times_;
  std::map<RenwuType, std::pair<int, int>> renwu_location_;
  RenwuType current_renwu_;
};
