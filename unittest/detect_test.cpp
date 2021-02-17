
#include "catch2/catch.hpp"
#include "object_detect/yolov5_detect.h"

TEST_CASE("detect", "yolov5") {
    cv::Mat image = cv::imread("image.png");
    REQUIRE_FALSE(image.empty());
    Yolov5Detect detect;
    REQUIRE(detect.InitWithFile("blhx-detect-config.yaml"));
    std::vector<ObjectBox> object_boxes = detect.Detect(image);
    for (const ObjectBox &box : object_boxes) {
        cv::Rect rect(box.x, box.y, box.width, box.height);
        cv::rectangle(image, rect, cv::Scalar(255, 0, 0),3);
        SPDLOG_INFO("{}: {} {} {} {}", box.name, box.x, box.y, box.width, box.height);
    }
    cv::imwrite("output.png", image);
}
