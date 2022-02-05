#pragma once

#include "image_input.h"
#include <fstream>

class FileImageInput : public IImageInput {
public:
    FileImageInput(const std::string &fname, const ImageFormat &image_format);

    bool Init() override;

    Image GetOneFrame() override;

private:
    std::string fname_;
    ImageFormat format_;
    std::vector<uint8_t> buffer_;
};
