/**
 * @file detect.h
 * @brief Base class for detection
 * @author peibozhao
 * @version 1.0.0
 * @date 2020-10-11
 */

#pragma once

#include "common/common.h"
#include <opencv2/core.hpp>
#include <string>
#include <vector>

struct ObjectBox {
  std::string class_name;
  RectangleI region;
};

class IObjectDetect {
public:
  virtual ~IObjectDetect() {}

  virtual bool Init() { return true; }

  virtual std::vector<ObjectBox> Detect(const cv::Mat &image) = 0;
};
