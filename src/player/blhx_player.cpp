
#include "blhx_player.h"
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

bool BLHXPlayer::Init(const std::string &cfg) {
  SPDLOG_INFO("Player config \n{}", cfg);
  try {
    YAML::Node config = YAML::Load(cfg);
    const YAML::Node &screen = config["screen"];
    screen_width_ = screen["width"].as<int>();
    screen_height_ = screen["height"].as<int>();
    std::string type = config["type"].as<std::string>();

    if (type == "battle") {
      BLHXBattleScence::Config bat_cfg = {
        .width =  screen_width_,
        .height =  screen_height_,
        .chapter_pattern =  config["chapter-pattern"].as<std::string>(),
        .name_pattern =  config["name-pattern"].as<std::string>(),
        .times = config["times"].as<int>()
      };
      scence_ = new BLHXBattleScence(bat_cfg);
    } else if (type == "yanxi") {
      BLHXYanxiScence::Config yanxi_cfg = {
        .width =  screen_width_,
        .height =  screen_height_,
      };
      scence_ = new BLHXYanxiScence(yanxi_cfg);
    }
  } catch (std::exception &e) {
    SPDLOG_ERROR("Catch exception. {}", e.what());
    return false;
  }
  return true;
}

PlayOperation BLHXPlayer::Play(const std::vector<DetectObject> &objs) {
  TimeLog time_log("Player");
  if (scence_->GetLimits()) {
    SPDLOG_INFO("Player get limites");
    return PlayOperation(PlayOperationType::LIMITS);
  }
  return scence_->ScencePlay(objs);
}

BLHXYanxiScence::BLHXYanxiScence(const Config &cfg) {
  width_ = cfg.width;
  height_ = cfg.height;
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
      ret.type = PlayOperationType::SCREEN_CLICK;
      ret.click.x = 400;
      ret.click.y = 400;
    } else if (obj.name == "出击") {
      ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
    } else if (obj.name == "演习" && obj.xmin > width_ / 2 && obj.ymin > height_ / 2) {
      ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
    } else if (std::regex_match(obj.name, std::regex("开始.*"))) {
      ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
    } else if (std::regex_match(obj.name, std::regex("点击.*"))) {
      // 点击继续 点击关闭
      ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
    } else if (obj.name == "确定") {
      ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj));
    }
  }
  return ret;
}

bool BLHXYanxiScence::GetLimits() {
  SPDLOG_INFO("Continuous chuji: {}", continuous_chuji_nums_);
  return continuous_chuji_nums_ >= 3;
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
  for (const auto &obj : objs) {
#define PLAY_CENTER(prio) \
      if (priority < prio) { \
        ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, BoxCenter(obj)); \
        priority = prio; \
      }
    if (std::regex_match(obj.name, chapter_reg_)) {
      // 点击章节名
      PLAY_CENTER(9)
    } else if (obj.name == "点击继续") {
      // 失败界面 点击继续
      PLAY_CENTER(10)
    } else if (std::regex_match(obj.name, std::regex("点击关.*"))) {
      // 失败界面
      SPDLOG_WARN("Defeat");
      PLAY_CENTER(10)
    } else if (obj.name == "锁定" || obj.name == "分享" || obj.name == "检视") {
      // 获得角色
      if (priority < 10) {
        ret = CreatePlayOperation(PlayOperationType::SCREEN_CLICK, {1000, 500});
        priority = 10;
      }
    } else if (obj.name == "立刻前往") {
      Reset();
      PLAY_CENTER(10)
    } else if (obj.name == "点击继续") {
      PLAY_CENTER(10)
    } else if (obj.name == "确定") {
      PLAY_CENTER(10)
    } else if (std::regex_match(obj.name, name_reg_)) {
      PLAY_CENTER(1)
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
  return left_times_ <= 0;
}

BLHXBattleScence::BLHXBattleScence(const Config &cfg) {
  boss_appeared_ = false;
  width_ = cfg.width;
  height_ = cfg.height;
  boundary_width_ = std::max(width_, height_) * 0.1;
  chapter_reg_.assign(cfg.chapter_pattern);
  name_reg_.assign(cfg.name_pattern);
  left_times_ = cfg.times;
}

PlayOperation BLHXBattleScence::Battle(const std::vector<DetectObject> &objs) {
  PlayOperation ret;
  if (boss_appeared_) {
    ret = AttackOneEnemy(objs, "enemy-boss");
  } else {
    // boss没有出现, 进攻普通敌人
    ret = AttackOneEnemy(objs, "enemy-normal");
  }
  ret = CheckBoundray(ret);
  return ret;
}

void BLHXBattleScence::Reset() {
  SPDLOG_DEBUG("Battle scence reset");
  left_times_ -= 1;
  SPDLOG_DEBUG("Battle left {}", left_times_);
  boss_appeared_ = false;
}

PlayOperation BLHXBattleScence::AttackOneEnemy(const std::vector<DetectObject> &objs, const std::string &name) {
  for (auto &obj : objs) {
    if (obj.name == name) {
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

PlayOperation BLHXBattleScence::CheckBoundray(const PlayOperation &opt) {
  if (opt.type == PlayOperationType::SCREEN_CLICK &&
      (opt.click.x < boundary_width_ ||
       opt.click.x > width_ - boundary_width_ ||
       opt.click.y < boundary_width_ ||
       opt.click.y > height_ - boundary_width_)) {
    SPDLOG_DEBUG("Click boundary: {} {}", opt.click.x, opt.click.y);
    // 点击点在边界, 需要滑动窗口
    PlayOperation ret;
    ret.type = PlayOperationType::SCREEN_SWIPE;
    ret.swipe.delta_x = width_ / 2 - opt.click.x;
    ret.swipe.delta_y = height_ / 2 - opt.click.y;
    return ret;
  }
  return opt;
}
