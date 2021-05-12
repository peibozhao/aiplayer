#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "common/common_types.h"

struct TextBox {
    int x, y, width, height;
    std::string text;

    TextBox() : x(0), y(0), width(0), height(0) {}
    TextBox(int xl, int yt, int w, int h, const std::string &t) : x(xl), y(yt), width(w), height(h), text(t) {}
};

class IOcrDetect {
public:
    virtual ~IOcrDetect() {}

    virtual bool Init(const std::string &config_str) { return true; }

    virtual bool SetParam(const std::string &key, const std::string &value) { return true; }

    virtual std::vector<TextBox> Detect(const cv::Mat &image) = 0;
};
