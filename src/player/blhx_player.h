#ifndef PLAYER_BLHX_PLAYER_H
#define PLAYER_BLHX_PLAYER_H

#include "player.h"
#include <map>

///< 碧蓝航线的情境
class IBLHXScence {
public:
  virtual PlayOperation ScencePlay(const std::vector<DetectObject> &objs) = 0;

  ///< @brief 是否到达限制了. 比如演习没有次数, 出击没有石油
  virtual bool GetLimits() { return false; }
};

class BLHXPlayer : public IPlayer {
public:
  bool Init(const std::string &cfg) override;
  PlayOperation Play(const std::vector<DetectObject> &objs) override;

private:
  PlayOperationType GetOperaionTypeByString(const std::string &opt);

  ///< @brief 是否在战场页面
  bool IsBattlefield(const std::vector<DetectObject> &objs);

private:
  std::vector<std::pair<std::string, PlayOperation>> operations_;
  int screen_width_, screen_height_;
  IBLHXScence *scence_;
};

#endif // ifndef PLAYER_BLHX_PLAYER_H
