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
#include <vector>

struct DetectObject {
  int xmin, xmax, ymin, ymax;
  float conf;
  std::string name;

  DetectObject() :
    xmin(0), xmax(0), ymin(0), ymax(0), conf(0.0) {
  }
};

enum class PlayOperationType {
  NONE,  // 没有操作
  SCREEN_CLICK,  // 触屏点击操作
  SCREEN_SWIPE,  // 触屏滑动操作
  LIMITS,  // 特殊符号. 到达限制
};

/// @brief 触屏点击操作
struct ClickOperation {
  int x, y;

  ClickOperation() : x(-1), y(-1) {}
};

/// @brief 触屏滑动操作
struct SwipeOperation {
  int delta_x, delta_y;

  SwipeOperation() : delta_x(-1), delta_y(-1) {}
};

/// @brief 操作
struct PlayOperation {
  PlayOperationType type;  // 操作类型
  union {
    ClickOperation click;
    SwipeOperation swipe;
  };

  PlayOperation(PlayOperationType type = PlayOperationType::NONE) {
    this->type = type;
  }
};

/**
 * @brief 输入检测结果,判断操作
 */
class IPlayer {
public:
  virtual bool Init(const std::string &cfg) = 0;

  virtual std::vector<PlayOperation> Play(const std::vector<DetectObject> &boxes) = 0;
};

#endif // ifndef PLAYER_PLAYER_H

