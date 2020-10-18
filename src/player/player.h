/**
 * @file player.h
 * @brief 判断操作
 * @author peibozhao
 * @version 1.0.0
 * @date 2020-10-13
 */

#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include <string>
#include "object_detect/detect.h"

enum class PlayOperationType {
  NONE,  // 没有操作
  SCREEN_CLICK  // 触屏点击操作
};

/// @brief 触屏点击操作
struct ClickOperation {
  int x, y;
};

/// @brief 操作
struct PlayOperation {
  PlayOperationType type;  // 操作类型
  union {
    ClickOperation click;
  };

  PlayOperation() {
    type = PlayOperationType::NONE;
  }
};

/**
 * @brief 输入检测结果,判断操作
 */
class IPlayer {
public:
  virtual bool Init(const std::string &cfg) = 0;
  virtual PlayOperation Play(const std::vector<DetectBox> &boxes) = 0;
};

#endif // ifndef PLAYER_PLAYER_H

