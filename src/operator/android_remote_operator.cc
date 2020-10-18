
#include "android_remote_operator.h"
#include "fmt/format.h"

bool AndroidRemoteOperator::Init(const std::string &cfg) {
  click_cmd_ = "adb shell input tap {} {}";
  return true;
}

void AndroidRemoteOperator::Click(int x, int y) {
  std::string click_cmd = fmt::format(click_cmd_, x, y);
  system(click_cmd.c_str());
}

