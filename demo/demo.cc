
#include <thread>
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "yaml-cpp/yaml.h"
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
  // auto logger = spdlog::basic_logger_mt("filelogger", "output");
  auto logger = spdlog::stdout_color_mt("stdout");
  logger->set_level(spdlog::level::debug);
  // logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^%l%$: %v");
  spdlog::set_default_logger(logger);

  AndroidRemoteScreenshotReader reader;
  if (!reader.Init(R"(
  remote_filename: /sdcard/temp/img.png
  local_filename: /tmp/img.png
  )")) {
    return -1;
  }

  YAML::Node net_config = YAML::LoadFile("blhx-v2.0.0.yaml");
  net_config["image_width"] = 2340;
  net_config["image_height"] = 1080;

  Yolov5Detect detect;
  if (!detect.Init(YAML::Dump(std::move(net_config)))) {
    return -1;
  }
  BLHXPlayer player;
  if (!player.Init("")) {
    return -1;
  }
  AndroidRemoteOperator operate;
  if (!operate.Init("")) {
    return -1;
  }

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
      bool opt_ret = operate.Operator(opt);
      if (!opt_ret) {
        spdlog::error("Operate failed.");
        break;
      }
      cv::Mat cv_img = CombineImage(img, boxes, opt);
      cv::imwrite("output.jpg", cv_img);
    } else {
      spdlog::error("Read image failed.");
      break;
    }
  }
}
