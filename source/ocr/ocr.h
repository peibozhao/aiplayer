#pragma once

#include "common/common.h"
#include <opencv2/core.hpp>
#include <string>
#include <vector>

struct TextBox {
  std::string text;
  RectangleI region;
};

class IOcrDetect {
public:
  virtual ~IOcrDetect() {}

  virtual bool Init() { return true; }

  virtual std::vector<TextBox> Detect(const cv::Mat &image) = 0;
};
