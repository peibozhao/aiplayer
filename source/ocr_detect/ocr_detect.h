#pragma once

#include <string>
#include <vector>
#include "common/common_types.h"

struct TextBox {
    float xmin, xmax, ymin, ymax;
    float conf;
    std::string text;

    TextBox() : xmin(0), xmax(0), ymin(0), ymax(0), conf(0.0) {}
    TextBox(float xi, float yi, float xa, float ya, float c, const std::string &t) : xmin(xi), xmax(xa), ymin(yi), ymax(ya), conf(c), text(t) {}
};

class IOcrDetect {
public:
    virtual ~IOcrDetect() {}

    virtual bool Init(const std::string &cfg) = 0;

    virtual bool SetParam(const std::string &key, const std::string &value) { return true; }

    virtual std::vector<TextBox> Detect(const Image &image) = 0;
};
