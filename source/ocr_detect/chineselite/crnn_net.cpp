
#include "crnn_net.h"
#include "glog/logging.h"
#include <algorithm>
#include <fstream>
#include <numeric>

bool CrnnNet::InitModel(const std::string &model_fname) {
    mnn_net_.reset(MNN::Interpreter::createFromFile(model_fname.c_str()));
    if (mnn_net_ == nullptr) {
        LOG(ERROR) << "MNN read net";
        return false;
    }
    MNN::ScheduleConfig mnn_config;
    mnn_session_ = mnn_net_->createSession(mnn_config);
    if (mnn_session_ == nullptr) {
        LOG(ERROR) << "MNN create session";
        return false;
    }
    return true;
}

bool CrnnNet::InitKeys(const std::string &keys_fname) {
    keys_.clear();
    std::ifstream ifs(keys_fname);
    if (!ifs.is_open()) {
        return false;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        keys_.emplace_back(line);
    }
    return true;
}

bool CrnnNet::InitConfig(const Config &config) {
    if (keys_.empty()) {
        LOG(ERROR) << "Init keys first";
        return false;
    }
    for (int i = 0; i < keys_.size(); ++i) {
        for (const auto &hot : config.hot_word) {
            if (hot == keys_[i]) {
                hot_keys_.insert(i + 1);
            }
        }
    }
    hot_scale_ = config.hot_scale;
    charscore_thresh_ = config.word_thresh;
    return true;
}

std::string CrnnNet::Detect(const cv::Mat &image) {
    float scale = (float)dst_height_ / (float)image.rows;
    int dst_width = int((float)image.cols * scale);

    cv::Mat image_resize;
    cv::resize(image, image_resize, cv::Size(dst_width, dst_height_));

    MNN::Tensor *mnn_input = mnn_net_->getSessionInput(mnn_session_, nullptr);
    std::vector<int> input_shape{1, image_resize.channels(), image_resize.rows, image_resize.cols};
    mnn_net_->resizeTensor(mnn_input, input_shape);
    mnn_net_->resizeSession(mnn_session_);

    auto batch = mnn_input->batch();
    auto channel = mnn_input->channel();
    auto height = mnn_input->height();
    auto width = mnn_input->width();

    MNN::Tensor *nchw_input = MNN::Tensor::create<float>(input_shape, nullptr, MNN::Tensor::CAFFE);
    for (int ch = 0; ch < 3; ++ch) {
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                nchw_input->host<float>()[ch * height * width + h * width + w] =
                    (image_resize.data[h * image_resize.cols * image_resize.channels() +
                                       w * image_resize.channels() + ch] -
                     mean_[ch]) *
                    norm_[ch];
            }
        }
    }
    mnn_input->copyFromHostTensor(nchw_input);
    mnn_net_->runSession(mnn_session_);

    MNN::Tensor *mnn_output = mnn_net_->getSessionOutput(mnn_session_, "out");
    std::vector<int> output_shape = mnn_output->shape();
    int64_t output_count =
        std::accumulate(output_shape.begin(), output_shape.end(), 1, std::multiplies<int>());
    MNN::Tensor *nchw_output =
        MNN::Tensor::create<float>(output_shape, nullptr, MNN::Tensor::CAFFE);
    mnn_output->copyToHostTensor(nchw_output);

    float *output_buffer = nchw_output->host<float>();
    std::vector<std::vector<float>> output(output_shape[0]);
    for (int i = 0; i < output_shape[0]; ++i) {
        output[i].resize(output_shape[2]);
        memcpy(output[i].data(), output_buffer + i * output_shape[2],
               output_shape[2] * sizeof(float));
    }
    return PostProcess(output);
}

std::string CrnnNet::PostProcess(const std::vector<std::vector<float>> &output) {
    int key_size = keys_.size();
    assert(key_size == output[0].size());
    std::string res;
    int last_index = 0;
    int max_index;
    float max_value;

    for (int i = 0; i < output.size(); i++) {
        const std::vector<float> &oneword_output = output[i];
        int max_index = 0;
        float max_value = -1000.f;
        // Do softmax
        std::vector<float> exps(oneword_output.size());
        for (int j = 0; j < oneword_output.size(); j++) {
            exps[j] = std::exp(oneword_output[j]);
            if (hot_keys_.find(j) != hot_keys_.end()) {
                exps[j] *= hot_scale_;
            }
        }
        float partition = std::accumulate(exps.begin(), exps.end(), 0.0); // row sum
        max_index = std::max_element(exps.begin(), exps.end()) - exps.begin();
        max_value = float(*std::max_element(exps.begin(), exps.end())) / partition;
        if (max_index > 0 && max_index < key_size) {
            if (max_value > charscore_thresh_ && (!(i > 0 && max_index == last_index))) {
                res.append(keys_[max_index - 1]);
            }
        }
        last_index = max_index;
    }
    return res;
}
