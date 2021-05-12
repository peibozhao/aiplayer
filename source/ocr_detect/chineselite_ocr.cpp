
#include "chineselite_ocr.h"
#include <fstream>
#include <opencv4/opencv2/opencv.hpp>
#include "yaml-cpp/yaml.h"
#include "glog/logging.h"

bool ChineseOcr::Init(const std::string &config_str) {
    std::string dbnet_fname;
    std::string crnn_fname;
    std::string keys_fname;
    CrnnNet::Config crnn_config;
    try {
        YAML::Node config = YAML::Load(config_str);
        dbnet_fname = config["dbnet"]["model"].as<std::string>();
        crnn_fname = config["crnn"]["model"].as<std::string>();
        keys_fname = config["crnn"]["keys"].as<std::string>();
        crnn_config.hot_word = config["crnn"]["hot_keys"].as<std::vector<std::string>>();
        crnn_config.hot_scale = config["crnn"]["hot_scale"].as<float>();
        crnn_config.word_thresh = config["crnn"]["thresh"].as<float>();
    } catch (std::exception &e) {
        LOG(ERROR) << "Catch error: " << e.what();
        return false;
    }
    db_net_.reset(new DbNet());
    if (!db_net_->InitModel(dbnet_fname)) {
        LOG(ERROR) << "Dbnet init failed";
        return false;
    }
    crnn_net_.reset(new CrnnNet());
    if (!crnn_net_->InitModel(crnn_fname)) {
        LOG(ERROR) << "Crnn init model failed";
        return false;
    }
    if (!crnn_net_->InitKeys(keys_fname)) {
        LOG(ERROR) << "Crnn init keys failed";
        return false;
    }
    if (!crnn_net_->InitConfig(crnn_config)) {
        LOG(ERROR) << "Crnn init config";
        return false;
    }
    maxside_len_ = 1024;
    return true;
}

std::vector<TextBox> ChineseOcr::Detect(const cv::Mat &image) {
    cv::Mat cv_image;
    cv::cvtColor(image, cv_image, cv::COLOR_BGR2RGB);
    int origin_maxside = std::max(cv_image.cols, cv_image.rows);
    int resize;
    if (maxside_len_ <= 0 || maxside_len_ > origin_maxside) {
        resize = origin_maxside;
    } else {
        resize = maxside_len_;
    }

    cv::Size dst_size;
    dst_size.width = cv_image.cols;
    dst_size.height = cv_image.rows;
    float ratio = 1.f;
    if (cv_image.rows > cv_image.cols) {
        ratio = float(maxside_len_) / float(cv_image.rows);
    } else {
        ratio = float(maxside_len_) / float(cv_image.cols);
    }
    dst_size.width = int(float(cv_image.cols) * ratio);
    dst_size.height = int(float(cv_image.rows) * ratio);
    if (dst_size.width % 32 != 0) {
        dst_size.width = (dst_size.width / 32) * 32;
        dst_size.width = std::max(dst_size.width, 32);
    }
    if (dst_size.height % 32 != 0) {
        dst_size.height = (dst_size.height / 32) * 32;
        dst_size.height = std::max(dst_size.height, 32);
    }
    float ratio_width = (float) dst_size.width / (float) cv_image.cols;
    float ratio_height = (float) dst_size.height / (float) cv_image.rows;

    cv::resize(cv_image, cv_image, dst_size);
    std::vector<Box> boxes = db_net_->Detect(cv_image);

    std::vector<TextBox> text_boxes;
    for (const Box &box : boxes) {
        cv::Rect cv_rect(box.x, box.y, box.width, box.height);
        cv::Mat part_image = cv_image(cv_rect);
        std::string text = crnn_net_->Detect(part_image);
        if (text.empty()) {
            continue;
        }
        float width_ratio = float(image.cols) / float(cv_image.cols);
        float height_ratio = float(image.rows) / float(cv_image.rows);
        TextBox text_box{int(box.x * width_ratio),
                         int(box.y * height_ratio),
                         int(box.width * width_ratio),
                         int(box.height * height_ratio),
                         text};
        text_boxes.emplace_back(text_box);
    }
    return text_boxes;
}
