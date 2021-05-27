
#include "catch2/catch.hpp"
#include "glog/logging.h"
#include "ocr_detect/chineselite/crnn_net.h"
#include "ocr_detect/chineselite/db_net.h"
#include "ocr_detect/chineselite_ocr.h"

TEST_CASE("chineselite", "ocr") {
    cv::Mat image = cv::imread("image.png");
    REQUIRE_FALSE(image.empty());
    ChineseOcr ocr;
    std::string config_str = R"(
dbnet:
    model: ../../models/dbnet.mnn
crnn:
    model: ../../models/crnn_lite_lstm.mnn
    keys: ../../models/keys.txt
    hot_keys: [切, 换, 立, 刻, 前, 往, 继, 续, 点, 关, 闭, 锁, 分, 享, 检, 视, 确, 定, 出, 击, 撤, 退]
    hot_scale: 100
    thresh: 0.6)";
    REQUIRE(ocr.Init(config_str));
    std::vector<TextBox> text_boxes = ocr.Detect(image);
    for (const TextBox &box : text_boxes) {
        cv::Rect rect(box.x, box.y, box.width, box.height);
        cv::rectangle(image, rect, cv::Scalar(255, 0, 0), 3);
        LOG(INFO) << box.text << ": " << box.x << "," << box.y << ","
                  << box.width << "," << box.height;
    }
    cv::imwrite("output.png", image);
}

TEST_CASE("dbnet", "ocr") {
    cv::Mat image = cv::imread("image.png");
    cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
    int maxside_len = 1024;
    int origin_maxside = std::max(image.cols, image.rows);
    int resize;
    if (maxside_len > origin_maxside) {
        resize = origin_maxside;
    } else {
        resize = maxside_len;
    }

    cv::Size dst_size;
    dst_size.width = image.cols;
    dst_size.height = image.rows;
    float ratio = 1.f;
    ratio = float(maxside_len) /
            float(image.rows > image.cols ? image.rows : image.cols);
    dst_size.width = int(float(image.cols) * ratio);
    dst_size.height = int(float(image.rows) * ratio);
    if (dst_size.width % 32 != 0) {
        dst_size.width = (dst_size.width / 32) * 32;
        dst_size.width = std::max(dst_size.width, 32);
    }
    if (dst_size.height % 32 != 0) {
        dst_size.height = (dst_size.height / 32) * 32;
        dst_size.height = std::max(dst_size.height, 32);
    }

    cv::resize(image, image, dst_size);
    REQUIRE_FALSE(image.empty());
    DbNet dbnet;
    REQUIRE(dbnet.InitModel("../../models/dbnet.mnn"));
    std::vector<Box> boxes = dbnet.Detect(image);
    for (int i = 0; i < boxes.size(); ++i) {
        const Box &box = boxes[i];
        cv::Rect rect(box.x, box.y, box.width, box.height);
        cv::rectangle(image, rect, cv::Scalar(255, 0, 0), 1);
        cv::putText(image, std::to_string(i), cv::Point(box.x, box.y),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
        LOG(INFO) << i << ": " << rect.x << "," << rect.y << "," << rect.width
                  << "," << rect.height;
    }
    cv::imwrite("dbnet_output.png", image);
}

TEST_CASE("crnn", "ocr") {
    cv::Mat image = cv::imread("dbnet_output.png");
    cv::Rect rect(657, 323, 94, 25);
    REQUIRE_FALSE(image.empty());
    image = image(rect);
    REQUIRE_FALSE(image.empty());
    cv::imwrite("crnn_input.png", image);
    CrnnNet crnn;
    REQUIRE(crnn.InitModel("crnn_lite_lstm.mnn"));
    REQUIRE(crnn.InitKeys("keys.txt"));
    CrnnNet::Config crnn_config;
    crnn_config.hot_scale = 10.f;
    crnn_config.hot_word = {"切", "换", "立", "刻", "前", "往", "继", "续",
                            "点", "关", "闭", "锁", "分", "享", "检", "视",
                            "确", "定", "出", "击", "撤", "退"};
    crnn_config.word_thresh = 0.2;
    REQUIRE(crnn.InitConfig(crnn_config));
    std::string text = crnn.Detect(image);
    LOG(INFO) << text;
}
