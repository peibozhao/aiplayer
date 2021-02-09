
#include "chineselite_ocr.h"
#include <opencv4/opencv2/opencv.hpp>


bool ChineseLiteOcr::Init(const std::string &cfg) {
    db_net_.reset(new DbNet());
    db_net_->InitModel("/home/peibozhao/Code/test/dbnet_changed.mnn");
    padding_ = 50;
    maxside_len_ = 1024;
    return true;
}

std::vector<TextBox> ChineseLiteOcr::Detect(const Image &/*image*/) {
    cv::Mat cv_image = cv::imread("image.png");
    cv::cvtColor(cv_image, cv_image, cv::COLOR_BGR2RGB);
    int origin_maxside = std::max(cv_image.cols, cv_image.rows);
    int resize;
    if (maxside_len_ <= 0 || maxside_len_ > origin_maxside) {
        resize = origin_maxside;
    } else {
        resize = maxside_len_;
    }
    resize += 2 * padding_;

    if (padding_ > 0) {
        cv::Scalar padding_scalar = {255, 255, 255};
        cv::copyMakeBorder(cv_image, cv_image, padding_, padding_, padding_, padding_,
                           cv::BORDER_ISOLATED, padding_scalar);
    }

    std::cout << cv_image.rows << " " << cv_image.cols << std::endl;
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

    std::cout << dst_size.width << " " << dst_size.height << std::endl;
    cv::resize(cv_image, cv_image, dst_size);
    std::vector<ChineseliteTextBox> boxes = db_net_->Detect(cv_image);

    std::vector<TextBox> text_boxes;
    for (const ChineseliteTextBox &box : boxes) {
        TextBox text_box{box.points[0].first,
                         box.points[0].second,
                         box.points[1].first,
                         box.points[1].second,
                         0.f,
                         "123"};
        text_boxes.emplace_back(text_box);
    }
    return text_boxes;
}
