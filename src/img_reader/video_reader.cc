
#include "video_reader.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"

bool VideoReader::Init(const std::string &cfg) {
  YAML::Node config = YAML::Load(cfg.c_str());
  video_width_ = config["width"].as<int>();
  video_height_ = config["height"].as<int>();
  video_fn_ = config["file"].as<std::string>();
  if (!video_capture_.open(video_fn_)) {
    spdlog::error("Open video failed. {}", video_fn_);
    return false;
  }
  return true;
}

bool VideoReader::Read(Image &img) {
  img.height = -1;
  img.width = -1;
  img.data.clear();
  cv::Mat cv_img;
  if (!video_capture_.read(cv_img)) {
    spdlog::error("Read image failed");
    return false;
  }

  img.width = video_width_;
  img.height = video_height_;
  img.data.resize(cv_img.rows * cv_img.cols * cv_img.channels());
  memcpy(img.data.data(), cv_img.data, img.data.size());
  return true;
}

