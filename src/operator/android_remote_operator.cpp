
#include "android_remote_operator.h"
#include "fmt/format.h"
#include "yaml-cpp/yaml.h"
#include "utils/util_functions.h"
#include "utils/util_types.h"

bool AndroidRemoteOperator::Init(const std::string &cfg) {
  click_cmd_ = "adb shell input tap {} {}";
  swipe_cmd_ = "adb shell input swipe {} {} {} {}";
  SPDLOG_INFO("Operator config: \n{}", cfg);
  try {
    YAML::Node config = YAML::Load(cfg);
    screen_width_ = config["screen"]["width"].as<int>();
    screen_height_ = config["screen"]["height"].as<int>();
  } catch (std::exception &e) {
    SPDLOG_ERROR("Catch error: {}", e.what());
    return false;
  }
  return true;
}

bool AndroidRemoteOperator::Click(int x, int y) {
  SPDLOG_INFO("Click {} {}", x, y);
  TimeLog time_log("Click");
  std::string click_cmd = fmt::format(click_cmd_, x, y);
  int click_ret = system(click_cmd.c_str());
  if (!HandleSystemResult(click_ret)) {
    SPDLOG_ERROR("Click failed");
    return false;
  }
  return true;
}

bool AndroidRemoteOperator::Swipe(int delta_x, int delta_y) {
  SPDLOG_INFO("Swipe {} {}", delta_x, delta_y);
  TimeLog time_log("Swipe");
  float swipe_len = std::sqrt(delta_x * delta_x + delta_y * delta_y);
  int origin_x = screen_width_ / 2, origin_y = screen_height_ / 2;

  int max_move_once = 500;
  int move_times = 0;
  while (move_times * max_move_once < swipe_len) {
    move_times += 1;
    int move_len = move_times * max_move_once > swipe_len
                       ? swipe_len - (move_times - 1) * max_move_once
                       : max_move_once;
    int move_x = float(move_len) / swipe_len * delta_x;
    int move_y = float(move_len) / swipe_len * delta_y;
    std::string swipe_cmd = fmt::format(swipe_cmd_, origin_x, origin_y,
                                        origin_x + move_x, origin_y + move_y);
    SPDLOG_DEBUG(swipe_cmd);
    int swipe_ret = system(swipe_cmd.c_str());
    if (!HandleSystemResult(swipe_ret)) {
      SPDLOG_ERROR("Click failed");
      return false;
    }
  }
  return true;
}
