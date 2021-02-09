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

struct ObjectBox {
    int xmin, xmax, ymin, ymax;
    float conf;
    std::string name;

    ObjectBox() : xmin(0), xmax(0), ymin(0), ymax(0), conf(0.0) {}
};

class IObjectDetect {
public:
    virtual ~IObjectDetect() {}

    virtual bool Init(const std::string &cfg) = 0;

    virtual bool SetParam(const std::string &key, const std::string &value) { return true; }

    virtual std::vector<ObjectBox> Detect(const std::vector<uint8_t> &image) = 0;
};
