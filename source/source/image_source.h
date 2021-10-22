
#pragma once

#include "source.h"
#include <string>

class ImageSource : public ISource {
public:
    ImageSource(const std::string &fname);

    bool Init() override;

    ImageFormat GetFormat() override;

    std::vector<char> GetImageBuffer() override;

private:
    std::string fname_;
    ImageFormat format_;
    std::vector<char> image_buffer_;
};
