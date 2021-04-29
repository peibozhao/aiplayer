
#include "catch2/catch.hpp"
#include "ocr_detect/chineselite_ocr.h"
#include "object_detect/yolov5_detect.h"
#include "blhx_player/battle_player.h"

TEST_CASE("battle", "blhx") {
    cv::Mat image = cv::imread("image.png");
    REQUIRE_FALSE(image.empty());
    ChineseOcr ocr;
    Yolov5Detect detect;
    BattlePlayer player;
    REQUIRE(ocr.InitWithFile("../../unittest/config/ocr.yaml"));
    REQUIRE(detect.InitWithFile("../../unittest/config/detect.yaml"));
    REQUIRE(player.InitWithFile("../../unittest/config/player.yaml"));
    std::vector<TextBox> text_boxes = ocr.Detect(image);
    for (const TextBox &box : text_boxes) {
        cv::Rect rect(box.x, box.y, box.width, box.height);
        cv::rectangle(image, rect, cv::Scalar(255, 0, 0),3);
        SPDLOG_INFO("Ocr {}: {} {} {} {}", box.text, box.x, box.y, box.width, box.height);
    }
    std::vector<ObjectBox> object_boxes = detect.Detect(image);
    for (const ObjectBox &box : object_boxes) {
        cv::Rect rect(box.x, box.y, box.width, box.height);
        cv::rectangle(image, rect, cv::Scalar(255, 0, 0),3);
        SPDLOG_INFO("Detect {}: {} {} {} {}", box.name, box.x, box.y, box.width, box.height);
    }
    std::vector<PlayOperation> operations = player.Play(object_boxes, text_boxes);
    for (const PlayOperation &opt : operations) {
        if (opt.type == PlayOperationType::SCREEN_CLICK) {
            cv::circle(image, cv::Point(opt.click.x, opt.click.y), 5, cv::Scalar(0, 0, 255), 5);
            SPDLOG_INFO("Play {}: {} {}", opt.type, opt.click.x, opt.click.y);
        } else if (opt.type == PlayOperationType::SCREEN_CLICK) {
            cv::Point start(image.cols / 2, image.rows / 2);
            cv::Point stop(start.x + opt.swipe.delta_x, start.y + opt.swipe.delta_y);
            cv::line(image, start, stop, cv::Scalar(0, 255, 0), 3);
            SPDLOG_INFO("Play {}: {} {}", opt.type, opt.swipe.delta_x, opt.swipe.delta_y);
        }
    }
    cv::imwrite("output.png", image);
}
