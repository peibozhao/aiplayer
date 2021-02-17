
#pragma once

#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include "MNN/Interpreter.hpp"

class CrnnNet {
public:
    bool InitModel(const std::string &model_fname);

    bool InitKeys(const std::string &keys_fname);

    std::string Detect(const cv::Mat &image);

private:
    bool CheckValid(const std::vector<float> &scores);

private:
    std::shared_ptr<MNN::Interpreter> mnn_net_;
    MNN::Session *mnn_session_;
    std::vector<std::string> keys_;
    const float mean_[3] = {127.5, 127.5, 127.5};
    const float norm_[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};
    const int dst_height_ = 32;
    const float charscore_thresh_ = 0.3f;
};

