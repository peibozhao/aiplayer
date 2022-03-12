
#pragma once

#include <opencv2/core.hpp>

class IImageInput {
public:
  virtual ~IImageInput(){};

  virtual bool Init() { return true; }

  virtual cv::Mat GetOneFrame() = 0;
};
