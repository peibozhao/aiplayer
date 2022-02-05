
#pragma once

#include <string>
#include <vector>

enum class ImageFormat {
    JPEG,
    YUV420
};

struct Image {
    ImageFormat format;
    std::vector<uint8_t> buffer;
};

// Read local image file
class IImageInput {
public:
    virtual ~IImageInput() {};

    virtual bool Init() { return true; }

    virtual Image GetOneFrame() = 0;
};
