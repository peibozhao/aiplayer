
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
    bool Init(std::istream &is) override;

    std::vector<PlayOperation> Play(const std::vector<ObjectBox> &object_boxes, const std::vector<TextBox> &text_boxes) override;

    bool GetLimit() override;

private:
  int width_, height_;
  std::vector<int> renwu_left_times_;
  std::map<RenwuType, std::pair<int, int>> renwu_location_;
  RenwuType current_renwu_;
};
