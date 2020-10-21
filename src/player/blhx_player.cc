
#include "blhx_player.h"
#include <regex>
#include "utils/util_types.h"
#include "yaml-cpp/yaml.h"

bool BLHXPlayer::Init(const std::string &cfg) {
  spdlog::info("Player config \n{}", cfg);
  same_operater_times_ = 0;
  try {
    YAML::Node config = YAML::Load(cfg);
    const YAML::Node &sp_opt_cfg = config["special-operation"];
    for (auto sp_cfg : sp_opt_cfg) {
      std::string label_name = sp_cfg.first.as<std::string>();
      PlayOperation play_opt;
      play_opt.type =
          GetOperaionTypeByString(sp_cfg.second["type"].as<std::string>());
      play_opt.click.x = sp_cfg.second["location"]["x"].as<int>();
      play_opt.click.y = sp_cfg.second["location"]["y"].as<int>();
      special_opeations_[label_name] = play_opt;
    }
  } catch (std::exception &e) {
    spdlog::error("Catch exception. {}", e.what());
    return false;
  }
  return true;
}

PlayOperation BLHXPlayer::Play(const std::vector<DetectBox> &boxes) {
  TimeLog time_log("Player");
  std::string cur_name;
  PlayOperation ret;
  ret.type = PlayOperationType::SCREEN_CLICK;
  ret.click.x = 1000;
  ret.click.y = 10;
  for (const auto &box : boxes) {
    cur_name = box.class_name;
    // 是否有特殊操作
    auto iter = special_opeations_.find(box.class_name);
    if (iter != special_opeations_.end()) {
      ret = iter->second;
      break;
    }

    std::smatch matchs;
    if (std::regex_search(box.class_name, matchs, std::regex("label"))) {
      // label
      ret.type = PlayOperationType::SCREEN_CLICK;
      ret.click.x = (box.xmin + box.xmax) / 2;
      ret.click.y = (box.ymin + box.ymax) / 2;
      break;
    } else if (std::regex_search(box.class_name, matchs, std::regex("enemy"))) {
      // enemy
      if (std::regex_search(box.class_name, matchs, std::regex("boss"))) {
        // boss
        if (same_operater_times_ >= 3 && last_name_ == box.class_name) {
          // 可能不能到达boss的位置
          spdlog::debug("Same operation. {} {}", last_name_, same_operater_times_);
          continue;
        }
        ret.type = PlayOperationType::SCREEN_CLICK;
        ret.click.x = (box.xmin + box.xmax) / 2;
        ret.click.y = (box.ymin + box.ymax) / 2;
        break;
      } else {
        // normal
        PlayOperation play_opt;
        play_opt.type = PlayOperationType::SCREEN_CLICK;
        play_opt.click.x = (box.xmin + box.xmax) / 2;
        play_opt.click.y = (box.ymin + box.ymax) / 2;
      }
    }
  }
  if (!cur_name.empty()) {
    same_operater_times_ = cur_name == last_name_ ? same_operater_times_ + 1 : 1;
    last_name_ = cur_name;
  }
  spdlog::info("Play {} {} {}", ret.type, ret.click.x, ret.click.y);
  return ret;
}

PlayOperationType BLHXPlayer::GetOperaionTypeByString(const std::string &opt) {
  if (opt == "click") {
    return PlayOperationType::SCREEN_CLICK;
  } else {
    return PlayOperationType::NONE;
  }
}
