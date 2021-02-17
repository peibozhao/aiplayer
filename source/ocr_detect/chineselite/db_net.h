
#pragma once

#include <memory>
#include "opencv2/opencv.hpp"
#include "MNN/Interpreter.hpp"
#include "common/common_types.h"

class DbNet {
public:
    bool InitModel(const std::string &model_fname);

    std::vector<Box> Detect(const cv::Mat &image);

private:
    std::shared_ptr<MNN::Interpreter> mnn_net_;
    MNN::Session *mnn_session_;
    float mean_[3] = {0.485 * 255, 0.456 * 255, 0.406 * 255};
    float norm_[3] = {1.0 / 0.229 / 255.0, 1.0 / 0.224 / 255.0, 1.0 / 0.225 / 255.0};
    float thresh_ = 0.3;
    float min_area_ = 50.f;
    float unclip_ratio_ = 2.f;
};

