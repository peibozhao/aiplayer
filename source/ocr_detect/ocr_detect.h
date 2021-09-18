#pragma once

#include "common/common_types.h"
#include <string>
#include <vector>

struct TextBox {
    int x, y, width, height;
    std::string text;

    TextBox() : x(0), y(0), width(0), height(0) {}

    TextBox(int xl, int yt, int w, int h, const std::string &t)
        : x(xl), y(yt), width(w), height(h), text(t) {}
};

class IOcrDetect {
public:
    virtual ~IOcrDetect() {}

    virtual bool Init() { return true; }

    virtual std::vector<TextBox> Detect(ImageFormat format,
                                        const std::vector<char> &buffer) = 0;
};
