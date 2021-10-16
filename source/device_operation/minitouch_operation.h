/**
 * @file minitouch_operation.h
 * @brief Operator by minitouch
 * @note Screen must be portrait at begin
 */

#pragma once

#include "device_operation.h"
#include <string>

struct ImageInfo {
    int width, height;
    int orientation;

    ImageInfo() : width(0), height(0), orientation(0) {
    }
};

class MinitouchOperation : public ITouchScreenOperation {
public:
    MinitouchOperation(const std::string &ip, unsigned short port,
                       const ImageInfo &image_info);

    ~MinitouchOperation() override;

    bool Init() override;

    void Click(int x, int y) override;

private:
    std::pair<int, int> TurnAroundPoint(int x, int y);

private:
    std::string ip_;
    unsigned short server_port_;
    int socket_;

    int width_, height_;
    int orientation_;
};
