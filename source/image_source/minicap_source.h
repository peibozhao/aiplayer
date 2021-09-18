
#pragma once

#include "image_source.h"
#include <string>

class MinicapSource : public IImageSource {
public:
    MinicapSource(const std::string &ip, unsigned short port);

    ~MinicapSource() override;

    bool Init() override;

    ImageFormat GetFormat() override;

    std::vector<char> GetImageBuffer() override;

private:
    std::string ip_;
    unsigned short server_port_;

    int socket_;
};

