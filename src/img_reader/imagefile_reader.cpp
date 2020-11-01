
#include "imagefile_reader.h"
#include <fstream>
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "utils/util_types.h"

#include "opencv2/imgcodecs.hpp"

bool ImageFileReader::Init(const std::string &cfg) {
  YAML::Node config = YAML::Load(cfg);
  img_width_ = config["width"].as<int>();
  img_height_ = config["height"].as<int>();
  filelist_ = config["file_list"].as<std::vector<std::string>>();
  cur_file_idx_ = 1;
  return true;
}

bool ImageFileReader::Read(Image &img) {
  TimeLog time_log;
  img.width = -1;
  img.height = -1;
  img.data.clear();
  cv::Mat cv_img = cv::imread(filelist_[cur_file_idx_++]);
  if (cv_img.empty()) {
    SPDLOG_ERROR("Read img failed. {}", filelist_[cur_file_idx_ - 1]);
    return false;
  }
  SPDLOG_DEBUG("Read. {}", filelist_[cur_file_idx_-1]);
  img.data.resize(cv_img.rows * cv_img.cols * cv_img.channels());
  memcpy(img.data.data(), cv_img.data, img.data.size());
  img.height = img_height_;
  img.width = img_width_;
  return true;
}
