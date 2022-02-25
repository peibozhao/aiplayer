#pragma once

#include <string>
#include "image_input.h"

class FileImageInput : public IImageInput {
public:
    FileImageInput(const std::string &fname);

    bool Init() override;

    cv::Mat GetOneFrame() override;

private:
    std::string fname_;
    cv::Mat image_;
};
