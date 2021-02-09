
#define CATCH_CONFIG_MAIN

#include <iostream>
#include "catch2/catch.hpp"
#include "ocr_detect/chineselite/db_net.h"
#include "ocr_detect/chineselite_ocr.h"

TEST_CASE("Debug", "DbNet") {
    ChineseLiteOcr ocr;
    ocr.Init("");
    ocr.Detect(Image());







    /*
    DbNet dbnet;
    std::cout << "-------------------------------" << std::endl;
    if (!dbnet.InitModel("/home/peibozhao/Code/test/dbnet_changed.mnn")) {
        std::cout << "db net init failed" << std::endl;
        return;
    }
    cv::Mat image = cv::imread("./img.png");
    // cv::resize(image, image, cv::Size(image.cols / 3, image.rows / 3));
    std::vector<ChineseliteTextBox> boxes = dbnet.Detect(image);
    for (const auto &box : boxes) {
        // std::cout << box.points[0].first << " " << box.points[0].second << std::endl;
        cv::Rect rect(box.points[0].first, box.points[0].second, box.points[1].first - box.points[0].first, box.points[1].second - box.points[0].second);
        cv::rectangle(image, rect, cv::Scalar(255, 0, 0), 5);
        // std::cout << box.points << std::endl;
    }
    cv::imwrite("output.png", image);
    */
}
