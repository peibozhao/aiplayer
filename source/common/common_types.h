
#pragma once

#include <vector>
#include <stdint.h>

struct Image {
    std::vector<uint8_t> data;
    int height, width;

    Image() {
        height = 0;
        width = 0;
    }
};
