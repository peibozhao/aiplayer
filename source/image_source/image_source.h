
#pragma once

#include <vector>
#include "common/common_types.h"

class IImageSource {
public:
    virtual ~IImageSource() {}

    virtual bool Init() { return true; }

    virtual ImageFormat GetFormat() = 0;

    virtual std::vector<char> GetImageBuffer() = 0;

};
