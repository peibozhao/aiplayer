
#include <thread>
#include "spdlog/spdlog.h"
#include "img_reader/video_reader.h"
#include "img_reader/imagefile_reader.h"
#include "img_reader/android_remote_screenshot_reader.h"
#include "object_detect/yolov5_detect.h"
#include "player/blhx_player.h"
#include "operator/android_remote_operator.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"

cv::Mat CombineImage(const Image &img, const std::vector<DetectBox> &boxes, const PlayOperation &opt) {
  cv::Mat ret = ImageToCvImage(img);
  for (auto box : boxes) {
    cv::Rect rect(box.xmin, box.ymin, box.xmax - box.xmin, box.ymax - box.ymin);
    cv::rectangle(ret, rect, cv::Scalar(255, 255, 255), 3);
    cv::putText(ret, box.class_name, cv::Point(box.xmax, box.ymin),
                cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 255, 255), 3);
  }
  cv::circle(ret, cv::Point(opt.click.x, opt.click.y), 5, cv::Scalar(0, 0, 255), 5);
  cv::circle(ret, cv::Point(opt.click.x, opt.click.y), 20, cv::Scalar(0, 0, 255), 5);
  return ret;
}

int main() {
  spdlog::set_level(spdlog::level::debug);

  AndroidRemoteScreenshotReader reader;
  reader.Init(R"(
  remote_filename: /sdcard/temp/img.png
  local_filename: /tmp/img.png
  )");
  Yolov5Detect detect;
  detect.Init(R"(
  image_width: 2340
  image_height: 1080
  network:
    input_width: 640
    input_height: 320
    class_names: [enemy-normal, enemy-boss, label-meirirenwu, label-likeqianwang, label-yingji, label-chuji, label-zhandoupingjia, label-dianjijixu, label-queding]
    score_thresh: 0.8
    nms_thresh: 0.8
    stride:
      output: 8
      1036: 16
      1056: 32
    anchor_grid:
      output: [[10, 13], [16, 30], [33, 23]]
      1036: [[30, 61], [62, 45], [59, 119]]
      1056: [[116, 90], [156, 198], [373, 326]]
    net_file: last.mnn
  )");
  BLHXPlayer player;
  player.Init("");
  AndroidRemoteOperator operate;
  operate.Init("");

  while (true) {
    // Wait animation
    std::this_thread::sleep_for(std::chrono::seconds(3));
    Image img;
    if (reader.Read(img)) {
      auto boxes = detect.Detect(img.data);
      for (const auto &box : boxes) {
        spdlog::info("Detect: {},{},{},{},{},{}",
                     box.class_name, box.xmin, box.ymin,
                     box.xmax, box.ymax, box.conf);
      }
      auto opt = player.Play(boxes);
      operate.Operator(opt);
      cv::Mat cv_img = CombineImage(img, boxes, opt);
      cv::imwrite("output.jpg", cv_img);
    } else {
      spdlog::error("Read image failed.");
      break;
    }
  }
}
