/**
 * @file minitouch_operation.h
 * @brief Operator by minitouch
 * @note Screen must be portrait at begin
 */

#pragma once

#include "device_operation.h"
#include <string>
#include "source/image/source.h"

class MinitouchOperation : public ITouchScreenOperation {
public:
    MinitouchOperation(const std::string &ip, unsigned short port, const ImageInfo &image_info,
                       int orientation, int delay_ms);

    ~MinitouchOperation() override;

    bool Init() override;

    void Click(int x, int y) override;

private:
    void ParseHeader(char *buffer, int len);

    std::pair<int, int> CoordinateConvertion(int x, int y);

    void Delay();

private:
    std::string ip_;
    unsigned short server_port_;
    int socket_;

    int image_width_, image_height_;
    int orientation_;
    int max_x_, max_y_;

    int delay_ms_;
};
