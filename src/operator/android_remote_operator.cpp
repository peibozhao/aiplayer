
#include "android_remote_operator.h"
#include "fmt/format.h"
#include "utils/util_functions.h"
#include "utils/util_types.h"

bool AndroidRemoteOperator::Init(const std::string &cfg) {
  click_cmd_ = "adb shell input tap {} {}";
  return true;
}

bool AndroidRemoteOperator::Click(int x, int y) {
  spdlog::info("Click {} {}", x, y);
  TimeLog time_log("Click");
  std::string click_cmd = fmt::format(click_cmd_, x, y);
  int click_ret = system(click_cmd.c_str());
  bool ret = HandleSystemResult(click_ret);
  if (!ret) {
    spdlog::error("Click failed");
  }
  return ret;
}

