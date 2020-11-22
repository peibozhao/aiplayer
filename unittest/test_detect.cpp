
#include <fstream>
#include <vector>
#include <iterator>
#include "catch2/catch.hpp"
#include "opencv2/opencv.hpp"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "object_detect/yolov5_detect.h"
#include "img_reader/reader.h"

TEST_CASE("Detect") {
  SECTION("Yolov5") {
    YAML::Node net_config = YAML::LoadFile("blhx-v3.0.0.yaml");
    net_config["image_width"] = 2340;
    net_config["image_height"] = 1080;
    Yolov5Detect *detect = new Yolov5Detect();
    REQUIRE(detect->Init(YAML::Dump(std::move(net_config))));
    cv::Mat cv_img = cv::imread("img.png");
    REQUIRE(!cv_img.empty());
    Image img = CvImageToImage(cv_img);
    std::ifstream ifs("./test.rgb", std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    std::vector<DetectBox> boxes = detect->Detect(data);
    for (const auto &box : boxes) {
      SPDLOG_INFO("{},{},{},{},{},{}\n", box.class_name, box.xmin, box.ymin, box.xmax, box.ymax, box.conf);
    }
  }
}
