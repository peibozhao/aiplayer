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
  int same_operater_times_;  /// 连续相同操作的次数
  std::string last_name_;  /// 上次点击的名称
  int normal_enemy_times_;  /// 连续点击普通敌人的次数
  std::map<std::string, PlayOperation> special_opeations_;
};

#endif // ifndef PLAYER_BLHX_PLAYER_H

