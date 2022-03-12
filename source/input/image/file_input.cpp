
#include "file_input.h"
#include "common/log.h"
#include <fstream>
#include <opencv2/imgcodecs.hpp>

FileImageInput::FileImageInput(const std::string &fname) { fname_ = fname; }

bool FileImageInput::Init() {
  std::ifstream ifs(fname_, std::ios::binary);
  if (!ifs.is_open()) {
    LOG(ERROR) << "File open failed: " << fname_;
    return false;
  }
  image_ = cv::imread(fname_);
  return !image_.empty();
}

cv::Mat FileImageInput::GetOneFrame() { return image_; }
