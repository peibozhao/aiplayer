
#include "blhx_player.h"
#include <regex>
#include "utils/util_types.h"
#include "yaml-cpp/yaml.h"

bool BLHXPlayer::Init(const std::string &cfg) {
  spdlog::info("Player config \n{}", cfg);
  same_operater_times_ = 0;
  try {
    YAML::Node config = YAML::Load(cfg);
    const YAML::Node &opts = config["operations"];
    for (auto opt : opts) {
      std::string label_name = opt.first.as<std::string>();
      PlayOperation play_opt;
      play_opt.type =
          GetOperaionTypeByString(opt.second["type"].as<std::string>());
      if (opt.second["location"].IsDefined()) {
        play_opt.click.x = opt.second["location"]["x"].as<int>();
        play_opt.click.y = opt.second["location"]["y"].as<int>();
      } else {
        play_opt.click.x = -1;
        play_opt.click.y = -1;
      }
      operations_.emplace_back(std::make_pair(label_name, play_opt));
    }
  } catch (std::exception &e) {
    spdlog::error("Catch exception. {}", e.what());
    return false;
  }
  return true;
}

PlayOperation BLHXPlayer::Play(const std::vector<DetectObject> &objs) {
  TimeLog time_log("Player");
  PlayOperation ret;
  for (const auto &play_opt : operations_) {
    for (const auto &obj : objs) {
      if (play_opt.first == obj.name) {
        ret = play_opt.second;
        ret.click.x = ret.click.x < 0 ? (obj.xmin + obj.xmax) / 2 : ret.click.x;
        ret.click.y = ret.click.y < 0 ? (obj.ymin + obj.ymax) / 2 : ret.click.y;
        goto end;
      }
    }
  }
end:
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
