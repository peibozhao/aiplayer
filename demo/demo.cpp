
#include <thread>
#include <fstream>
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "img_reader/video_reader.h"
#include "img_reader/imagefile_reader.h"
#include "img_reader/android_remote_screenshot_reader.h"
#include "object_detect/yolov5_detect.h"
#include "ocr/ocr_http_client.h"
#include "player/blhx_player.h"
#include "operator/android_remote_operator.h"
#include "utils/util_converter.h"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"

cv::Mat CombineImage(const Image &img, const std::vector<DetectObject> &objes, const PlayOperation &opt) {
  cv::Mat ret = ImageToCvImage(img);
  for (auto obj : objes) {
    cv::Rect rect(obj.xmin, obj.ymin, obj.xmax - obj.xmin, obj.ymax - obj.ymin);
    cv::rectangle(ret, rect, cv::Scalar(255, 255, 255), 3);
    cv::putText(ret, obj.name, cv::Point(obj.xmax, obj.ymin),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
  }
  cv::circle(ret, cv::Point(opt.click.x, opt.click.y), 5, cv::Scalar(0, 0, 255), 5);
  cv::circle(ret, cv::Point(opt.click.x, opt.click.y), 20, cv::Scalar(0, 0, 255), 5);
  return ret;
}

int main() {
  auto logger = spdlog::stdout_color_mt("stdout");
  logger->set_level(spdlog::level::debug);
  spdlog::set_default_logger(logger);

  int img_width = 2340;
  int img_height = 1080;
  AndroidRemoteScreenshotReader reader;
  if (!reader.Init(R"(
  remote_filename: /sdcard/temp/img.png
  local_filename: /tmp/img.png
  )")) {
    return -1;
  }

  YAML::Node net_config = YAML::LoadFile("blhx-v2.0.0.yaml");
  net_config["image_width"] = img_width;
  net_config["image_height"] = img_height;
  Yolov5Detect detect;
  if (!detect.Init(YAML::Dump(std::move(net_config)))) {
    return -1;
  }

  OCRHTTPClient ocr;
  if (!ocr.Init(fmt::format(R"(
  image_width: {}
  image_height: {}
  host: 127.0.0.1:8089
  path: /api/tr-run/
  )", img_width, img_height))) {
    return -1;
  }

  std::ifstream player_cfg_file("../../config/blhx-player.yaml");
  std::string player_cfg((std::istreambuf_iterator<char>(player_cfg_file)),
                         std::istreambuf_iterator<char>());
  BLHXPlayer player;
  if (!player.Init(player_cfg)) {
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
      std::vector<DetectObject> detect_objs;
      auto boxes = detect.Detect(img.data);
      for (const auto &box : boxes) {
        spdlog::info("Detect: {} {},{},{},{} {}",
                     box.class_name, box.xmin, box.ymin,
                     box.xmax, box.ymax, box.conf);
        detect_objs.emplace_back(Converter(box));
      }
      auto words = ocr.Detect(img.data);
      for (const auto &word : words) {
        spdlog::info("Detect: {} {},{},{},{} {}",
                     word.word, word.xmin, word.ymin,
                     word.xmax, word.ymax, word.conf);
        detect_objs.emplace_back(Converter(word));
      }
      auto opt = player.Play(detect_objs);
      bool opt_ret = operate.Operator(opt);
      if (!opt_ret) {
        spdlog::error("Operate failed.");
        break;
      }
      cv::Mat cv_img = CombineImage(img, detect_objs, opt);
      cv::imwrite("output.jpg", cv_img);
    } else {
      spdlog::error("Read image failed.");
      break;
    }
  }
}
