
#include "blhx_player.h"
#include <regex>
#include <random>
#include "utils/util_types.h"
#include "yaml-cpp/yaml.h"

static PlayOperation CreatePlayOperation(PlayOperationType type, std::vector<int> num) {
  PlayOperation ret;
  ret.type = type;
  switch (type) {
    case PlayOperationType::SCREEN_CLICK:
      ret.click.x = num[0];
      ret.click.y = num[1];
      break;
    case PlayOperationType::SCREEN_SWIPE:
      ret.swipe.delta_x = num[0];
      ret.swipe.delta_y = num[1];
      break;
    default:
      ret.type = PlayOperationType::NONE;
      break;
  }
  return ret;
}

static std::vector<int> BoxCenter(const DetectObject &obj) {
  return {(obj.xmin + obj.xmax) / 2, (obj.ymin + obj.ymax) / 2};
}

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
  BLHXBattleScence();
  PlayOperation ScencePlay(const std::vector<DetectObject> &objs) override;
  bool GetLimits() override;

private:
  ///< 每次进入章节前重置状态
  void Reset();
  PlayOperation Battle(const std::vector<DetectObject> &objs);
  ///< 选择一个敌人进攻, 如果视野中没有该类型的敌人, 会返回随机的滑动操作
  PlayOperation AttackOneEnemy(const std::vector<DetectObject> &objs, const std::string &name);

private:
  ///< boss是否出现了
  bool boss_appeared_;
};

bool BLHXPlayer::Init(const std::string &cfg) {
  SPDLOG_INFO("Player config \n{}", cfg);
  scence_ = new BLHXBattleScence();
  // scence_ = new BLHXYanxiScence();
  try {
    YAML::Node config = YAML::Load(cfg);
    const YAML::Node &screen = config["screen"];
    screen_width_ = screen["width"].as<int>();
    screen_height_ = screen["height"].as<int>();
  } catch (std::exception &e) {
    SPDLOG_ERROR("Catch exception. {}", e.what());
    return false;
  }
  return true;
}

PlayOperation BLHXPlayer::Play(const std::vector<DetectObject> &objs) {
  TimeLog time_log("Player");
  if (scence_->GetLimits()) {
    SPDLOG_WARN("Player get limites");
  }
  return scence_->ScencePlay(objs);
}

BLHXYanxiScence::BLHXYanxiScence() {
  continuous_chuji_nums_ = 0;
}

PlayOperation BLHXYanxiScence::ScencePlay(const std::vector<DetectObject> &objs) {
  int last_continuous_chuji_nums = continuous_chuji_nums_;
  for (auto &obj : objs) {
    if (obj.name == "出击") {
      continuous_chuji_nums_ += 1;
      break;
    }
  }
  continuous_chuji_nums_ = last_continuous_chuji_nums == continuous_chuji_nums_ ? 0 : continuous_chuji_nums_;

  PlayOperation ret;
  std::string click_label;
  for (auto &obj : objs) {
    if (obj.name == "军需补给") {
      click_label = "军需补给";
      ret.type = PlayOperationType::SCREEN_CLICK;
    } else if (obj.name == "出击") {
      click_label = "出击";
      ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
    } else if (std::regex_match(obj.name, std::regex("开始.*"))) {
      click_label = "开始演习";
      ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
    } else if (std::regex_match(obj.name, std::regex("点击.*"))) {
      // 点击继续 点击关闭
      click_label = "点击";
      ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
    } else if (obj.name == "确定") {
      click_label = "确定";
      ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
    }
  }

  if (click_label == "军需补给") {
    ret.click.x = 400;
    ret.click.y = 400;
  }
  return ret;
}

bool BLHXYanxiScence::GetLimits() {
  return continuous_chuji_nums_ > 5;
}

PlayOperation BLHXBattleScence::ScencePlay(const std::vector<DetectObject> &objs) {
  for (auto &obj : objs) {
    if (!boss_appeared_ && obj.name == "enemy-boss") {
      // boss刚出现, 前一个队伍一般弹药都没了, boss比较难打
      for (auto &obj : objs) {
        if (obj.name == "切换") {
          SPDLOG_DEBUG("Change team");
          boss_appeared_ = true;
          return CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
        }
      }
      // boss出现没有检测到 切换, 不进行任何操作, 重新检测
      SPDLOG_WARN("boss appeard, qiehuan not appeared");
      return CreatePlayOperation(PlayOperationType::NONE, {});
    }
  }

  PlayOperation ret;
  int priority = -1;
  for (auto &obj : objs) {
    if (std::regex_match(obj.name, std::regex(".*热情的.*"))) {
      // 点击章节名
      if (priority < 9) {
        ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
        priority = 9;
      }
    } else if (std::regex_match(obj.name, std::regex("点击.*"))) {
      // 失败界面
      SPDLOG_WARN("Defeat");
      if (priority < 10) {
        ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
        priority = 10;
      }
    } else if (obj.name == "立刻前往") {
      Reset();
      if (priority < 10) {
        ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
        priority = 10;
      }
    } else if (obj.name == "点击继续") {
      if (priority < 10) {
        ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
        priority = 10;
      }
    } else if (obj.name == "确定") {
      if (priority < 10) {
        ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
        priority = 10;
      }
    } else if (std::regex_match(obj.name, std::regex(".*老婆.*"))) {
      if (priority < 1) {
        ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
        priority = 1;
      }
    } else if (obj.name == "撤退" || obj.name == "切换") {
      if (priority < 10) {
        ret = Battle(objs);
        priority = 10;
      }
    }
  }
  return ret;
}

bool BLHXBattleScence::GetLimits() {
  // TODO 判断石油
  return false;
}

BLHXBattleScence::BLHXBattleScence() {
  boss_appeared_ = false;
}

PlayOperation BLHXBattleScence::Battle(const std::vector<DetectObject> &objs) {
  if (boss_appeared_) {
    return AttackOneEnemy(objs, "enemy-boss");
  } else {
    // boss没有出现, 进攻普通敌人
    return AttackOneEnemy(objs, "enemy-normal");
  }
}

void BLHXBattleScence::Reset() {
  SPDLOG_DEBUG("Battle scence reset");
  boss_appeared_ = false;
}

PlayOperation BLHXBattleScence::AttackOneEnemy(const std::vector<DetectObject> &objs, const std::string &name) {
  for (auto &obj : objs) {
    if (obj.name == name) {
      int click_x = (obj.xmin + obj.xmax) / 2;
      int click_y = (obj.ymin + obj.ymax) / 2;
      if (click_x < 100 || click_y < 100) {
        // 在这两个边界可能被多余的东西遮挡
        continue;
      }
      return CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
    }
  }
  // 视野中不存在, 需要滑动窗口
  int len = 1000;
  std::mt19937 rand_engine(time(0));
  std::uniform_real_distribution<float> rand_dis(0, 3.14 * 2);
  float radian = rand_dis(rand_engine);
  return CreatePlayOperation(
      PlayOperationType::SCREEN_SWIPE,
      {int(len * std::sin(radian)), int(len * std::cos(radian))});
}
