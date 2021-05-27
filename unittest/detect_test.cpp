
#include "catch2/catch.hpp"
#include "glog/logging.h"
#include "object_detect/yolov5_detect.h"

TEST_CASE("yolov5", "detect") {
    cv::Mat image = cv::imread("image.png");
    REQUIRE_FALSE(image.empty());
    Yolov5Detect detect;
    std::string config_str = R"(
image_height: 1176
image_width: 2400
network:
  model: ../../models/blhx_detect.mnn
  nms_thresh: 0.7
  score_thresh: 0.7
  input_width: 640
  input_height: 320
  class_names: [敌人, boss, 每日任务, 立刻前往, 迎击, 出击, 战斗评价, 点击继续, 确定, 船舰小图, 当前船舰, 弹药]
  stride:
    output: 8
    1036: 16
    1056: 32
  anchor_grid:
    output: [[10, 13], [16, 30], [33, 23]]
    1036: [[30, 61], [62, 45], [59, 119]]
    1056: [[116, 90], [156, 198], [373, 326]])";
    REQUIRE(detect.Init(config_str));
    std::vector<ObjectBox> object_boxes = detect.Detect(image);
    for (ObjectBox &box : object_boxes) {
        cv::Rect rect(box.x, box.y, box.width, box.height);
        cv::rectangle(image, rect, cv::Scalar(255, 0, 0), 3);
        LOG(INFO) << box.name << ": " << box.x << "," << box.y << ","
                  << box.width << "," << box.height;
    }
    cv::imwrite("output.png", image);
}
