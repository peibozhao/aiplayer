#ifndef PLAYER_BLHX_PLAYER_H
#define PLAYER_BLHX_PLAYER_H

#include "player.h"
#include <map>
#include <regex>

///< 碧蓝航线的情境
class IBLHXScence {
public:
  virtual PlayOperation ScencePlay(const std::vector<DetectObject> &objs) = 0;

  ///< @brief 是否到达限制了. 比如演习没有次数, 出击没有石油
  virtual bool GetLimits() { return false; }
};

///< 碧蓝航线
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

///< 演习模式
class BLHXYanxiScence : public IBLHXScence {
public:
  BLHXYanxiScence();
  PlayOperation ScencePlay(const std::vector<DetectObject> &objs) override;
  bool GetLimits() override;

private:
  int continuous_chuji_nums_;  // 连续识别出出击的次数
};

///< 刷图模式
class BLHXBattleScence : public IBLHXScence {
public:
  struct Config {
    int width, height;
    std::string chapter_pattern;
    std::string name_pattern;
    int times;
  };

public:
  BLHXBattleScence(const Config &cfg);
  PlayOperation ScencePlay(const std::vector<DetectObject> &objs) override;
  bool GetLimits() override;

private:
  ///< 每次进入章节前重置状态
  void Reset();
  PlayOperation Battle(const std::vector<DetectObject> &objs);
  ///< 选择一个敌人进攻, 如果视野中没有该类型的敌人, 会返回随机的滑动操作
  PlayOperation AttackOneEnemy(const std::vector<DetectObject> &objs, const std::string &name);
  ///< 检查边框
  PlayOperation CheckBoundray(const PlayOperation &opt);

private:
  ///< boss是否出现了
  bool boss_appeared_;
  int width_, height_;
  int boundary_width_;
  std::regex chapter_reg_, name_reg_;
  int left_times_;
};

#endif // ifndef PLAYER_BLHX_PLAYER_H
