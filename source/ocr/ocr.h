#pragma once

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include "common/common.h"

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
