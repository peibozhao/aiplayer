
#pragma once

#include "ocr_detect.h"

class PaddleOcr : public IOcrDetect {
public:
    PaddleOcr(const std::string &ip, unsigned short port);

    ~PaddleOcr() override;

    std::vector<TextBox> Detect(ImageFormat format,
                                const std::vector<char> &buffer) override;
};
