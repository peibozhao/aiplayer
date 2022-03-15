#pragma once

#include <string>
#include <vector>

struct Element {
  std::string name;
  float x, y;

  Element() : name(""), x(0.), y(0.) {}
  Element(const std::string &n, float ix, float iy) : name(n), x(ix), y(iy) {}
};

enum class PlayOperationType {
  SCREEN_CLICK = 1, // 触屏点击操作
  SLEEP,            // 等待
  OVER              // 游戏结束
};

struct ClickOperation {
  float x, y;

  ClickOperation(float x_center = 0, float y_center = 0)
      : x(x_center), y(y_center) {}
};

struct PlayOperation {
  PlayOperationType type;
  union {
    ClickOperation click;
    int sleep_ms;
  };

  PlayOperation() {}
};

class IPlayer {
public:
  virtual ~IPlayer() {}

  IPlayer(const std::string &name) { name_ = name; }

  virtual bool Init() { return true; };

  /// @brief 通过检测结果返回行为
  virtual std::vector<PlayOperation>
  Play(const std::vector<Element> &elements) = 0;

  virtual std::string Name() { return name_; }

  /// @brief 设置mode
  virtual bool SetMode(const std::string &mode_name) { return false; }

  /// @brief 当前mode
  virtual std::string GetMode() { return ""; }

private:
  std::string name_;
};
