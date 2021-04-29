
#pragma once

#include <string>
#include <vector>
#include <tuple>
#include "opencv2/opencv.hpp"
#include "MNN/Interpreter.hpp"

class CrnnNet {
public:
    struct Config {
        std::vector<std::string> hot_word;
        float hot_scale;
        float word_thresh;
    };

public:
    bool InitModel(const std::string &model_fname);

    bool InitKeys(const std::string &keys_fname);

    bool InitConfig(const Config &config);

    std::string Detect(const cv::Mat &image);

private:
    std::string PostProcess(const std::vector<std::vector<float>> &output);

private:
    std::shared_ptr<MNN::Interpreter> mnn_net_;
    MNN::Session *mnn_session_;
    std::vector<std::string> keys_;
    const float mean_[3] = {127.5, 127.5, 127.5};
    const float norm_[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};
    const int dst_height_ = 32;
    float charscore_thresh_ = 0.0f;
    std::set<int> hot_keys_;
    float hot_scale_;
};

