
#pragma once

#include "ocr_detect.h"

class PaddleOcr : public IOcrDetect {
public:
    PaddleOcr();

    ~PaddleOcr() override;

    std::vector<TextBox> Detect(ImageFormat format, const std::vector<char> &buffer) override;

};
