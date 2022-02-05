/**
 * @file detect.h
 * @brief Base class for detection
 * @author peibozhao
 * @version 1.0.0
 * @date 2020-10-11
 */

#pragma once

#include "input/image/image_input.h"
#include <string>
#include <vector>

struct ObjectBox {
    uint16_t x, y; // center
    uint16_t width, height;
    std::string name;

    ObjectBox() : x(0), y(0), width(0), height(0) {}

    ObjectBox(int x_center, int y_center, int w, int h, const std::string &n)
        : x(x_center), y(y_center), width(w), height(h), name(n) {}
};

class IObjectDetect {
public:
    virtual ~IObjectDetect() {}

    virtual bool Init() { return true; }

    virtual std::vector<ObjectBox> Detect(const Image &image) = 0;
};
