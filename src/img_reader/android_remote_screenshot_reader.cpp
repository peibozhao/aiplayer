
#include "android_remote_screenshot_reader.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "fmt/format.h"
#include "utils/util_functions.h"
#include "utils/util_types.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

bool AndroidRemoteScreenshotReader::Init(const std::string &cfg) {
  SPDLOG_INFO("Reader config: \n{}", cfg);
  try {
    YAML::Node config = YAML::Load(cfg);
    remote_storage_fn_ = config["remote_filename"].as<std::string>();
    local_storage_fn_ = config["local_filename"].as<std::string>();
  } catch (std::exception &e) {
    SPDLOG_ERROR("Catch exception. {}", e.what());
    return false;
  }
  screenshot_cmd_ = "adb shell screencap -p {}";
  pull_img_cmd_ = "adb pull {} {}";
  return true;
}

bool AndroidRemoteScreenshotReader::Read(Image &img) {
  TimeLog time_log("Reader");
  std::string screenshot_cmd = fmt::format(screenshot_cmd_, remote_storage_fn_);
  SPDLOG_DEBUG(screenshot_cmd);
  int screenshot_ret = system(screenshot_cmd.c_str());
  if (!HandleSystemResult(screenshot_ret)) {
    SPDLOG_ERROR("Screenshot failed.");
    return false;
  }
  std::string pull_image_cmd = fmt::format(pull_img_cmd_, remote_storage_fn_, local_storage_fn_);
  SPDLOG_DEBUG(pull_image_cmd);
  int pull_ret = system(pull_image_cmd.c_str());
  if (!HandleSystemResult(pull_ret)) {
    SPDLOG_ERROR("Pull failed.");
    return false;
  }

  cv::Mat cv_img = cv::imread(local_storage_fn_);
  if (cv_img.empty()) {
    SPDLOG_ERROR("Image read failed. {}", local_storage_fn_);
    return false;
  }
  cv::cvtColor(cv_img, cv_img, cv::COLOR_BGR2RGB);
  img.height = cv_img.rows;
  img.width = cv_img.cols;
  img.data.resize(img.height * img.width * 3);
  memcpy(img.data.data(), cv_img.data, img.data.size());
  return true;
}
