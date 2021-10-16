#pragma once

#include "common/common_types.h"
#include <string>
#include <vector>

struct TextBox {
    int x, y;  // center
    int width, height;
    std::string text;

    TextBox() : x(0), y(0), width(0), height(0) {}

    TextBox(int x_center, int y_center, int w, int h, const std::string &t)
        : x(x_center), y(y_center), width(w), height(h), text(t) {}
};

class IOcrDetect {
public:
    virtual ~IOcrDetect() {}

    virtual bool Init() { return true; }

    virtual std::vector<TextBox> Detect(ImageFormat format,
                                        const std::vector<char> &buffer) = 0;
};
