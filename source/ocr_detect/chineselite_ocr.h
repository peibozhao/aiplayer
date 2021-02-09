
#pragma once

#include "ocr_detect.h"
#include "chineselite/db_net.h"
#include "chineselite/crnn_net.h"

class ChineseLiteOcr : public IOcrDetect {
public:
    bool Init(const std::string &cfg) override;

    std::vector<TextBox> Detect(const Image &image) override;

private:
    std::shared_ptr<DbNet> db_net_;
    int padding_;
    int maxside_len_;
};

