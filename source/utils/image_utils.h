#pragma once

#include "input/image/image_input.h"

typedef std::pair<uint16_t, uint16_t> FrameSize;

std::pair<uint16_t, uint16_t> ImageSize(const Image &image);

Image Convert(const Image &image, ImageFormat dst_format);

