
#pragma once

#include "image_source.h"

class MinicapSource : public IImageSource {
public:
    MinicapSource(unsigned short port);

    ~MinicapSource() override;

    bool Init() override;

    ImageFormat GetFormat() override;

    std::vector<char> GetImageBuffer() override;

private:
    unsigned short server_port_;

    int socket_;
};

