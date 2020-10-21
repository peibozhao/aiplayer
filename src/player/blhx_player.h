#ifndef PLAYER_BLHX_PLAYER_H
#define PLAYER_BLHX_PLAYER_H

#include "player.h"
#include <map>

class BLHXPlayer : public IPlayer {
public:
  bool Init(const std::string &cfg) override;
  PlayOperation Play(const std::vector<DetectBox> &boxes) override;

private:
  PlayOperationType GetOperaionTypeByString(const std::string &opt);

private:
  int normal_enemy_times_;
  std::map<std::string, PlayOperation> special_opeations_;
};

#endif // ifndef PLAYER_BLHX_PLAYER_H

