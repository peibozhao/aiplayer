
#pragma once

#include <vector>

enum ImageFormat {
    JPEG
};

struct ImageInfo {
    ImageFormat format;
    int width, height;
};


class ISource {
public:
    virtual ~ISource() {}

    virtual bool Init() { return true; }

    virtual void Start() {}

    virtual void Stop() {}

    virtual ImageInfo GetImageInfo() = 0;

    virtual std::vector<char> GetImageBuffer() = 0;
};
