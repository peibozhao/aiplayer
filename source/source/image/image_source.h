
#pragma once

#include "source.h"
#include <string>

// Read local image file
class ImageSource : public ISource {
public:
    ImageSource(const std::string &fname, const ImageInfo &image_info);

    bool Init() override;

    ImageInfo GetImageInfo() override;

    std::vector<char> GetImageBuffer() override;

private:
    std::string fname_;
    ImageInfo image_info_;
    std::vector<char> image_buffer_;
};
