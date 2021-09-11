/**
 * @file detect.h
 * @brief Base class for detection
 * @author peibozhao
 * @version 1.0.0
 * @date 2020-10-11
 */

#pragma once

#include <string>
#include <vector>
#include "common/common_types.h"

struct ObjectBox {
    int x, y, width, height;
    std::string name;

    ObjectBox() : x(0), y(0), width(0), height(0) {}

    ObjectBox(int xl, int yt, int w, int h, const std::string &n)
        : x(xl), y(yt), width(w), height(h), name(n) {}

};

class IObjectDetect {
public:
    virtual ~IObjectDetect() {}

    virtual bool Init() { return true; }

    virtual std::vector<ObjectBox> Detect(ImageFormat format, const std::vector<char> &buffer) = 0;
};
