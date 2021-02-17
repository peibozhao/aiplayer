
#include "catch2/catch.hpp"
#include "ocr_detect/chineselite_ocr.h"

TEST_CASE("ocr", "chineselite") {
    cv::Mat image = cv::imread("image.png");
    REQUIRE_FALSE(image.empty());
    ChineseOcr ocr;
    REQUIRE(ocr.InitWithFile("chineseocr.yaml"));
    std::vector<TextBox> text_boxes = ocr.Detect(image);
    for (const TextBox &box : text_boxes) {
        cv::Rect rect(box.x, box.y, box.width, box.height);
        cv::rectangle(image, rect, cv::Scalar(255, 0, 0),3);
        SPDLOG_INFO("{}: {} {} {} {}", box.text, box.x, box.y, box.width, box.height);
    }
    cv::imwrite("output.png", image);
}
