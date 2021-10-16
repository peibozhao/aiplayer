
#pragma once

#include "ocr_detect.h"
#include "httplib.h"

class PaddleOcr : public IOcrDetect {
public:
    PaddleOcr(const std::string &host, unsigned short port);

    ~PaddleOcr() override;

    bool Init() override;

    std::vector<TextBox> Detect(ImageFormat format,
                                const std::vector<char> &buffer) override;

private:
    std::string host_;
    unsigned short port_;
    std::shared_ptr<httplib::Client> client_;
};
