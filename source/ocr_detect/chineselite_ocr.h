/**
 * @file chineselite_ocr.h
 * @brief Chinese ocr
 * @author peibozhao
 * @version 1.0.0
 * @date 2021-02-15
 */

#pragma once

#include <opencv2/opencv.hpp>
#include "ocr_detect.h"
#include "chineselite/db_net.h"
#include "chineselite/crnn_net.h"

/**
 * @brief Base on https://github.com/ouyanghuiyu/chineseocr_lite
 */
class ChineseOcr : public IOcrDetect {
public:
    bool Init(std::istream &is) override;

    std::vector<TextBox> Detect(const cv::Mat &image) override;

private:
    std::shared_ptr<DbNet> db_net_;
    std::shared_ptr<CrnnNet> crnn_net_;
    int maxside_len_;
};

