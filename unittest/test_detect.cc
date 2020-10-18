
#include <fstream>
#include <vector>
#include <iterator>
#include "catch2/catch.hpp"
#include "object_detect/yolov5_detect.h"
#include "spdlog/spdlog.h"

TEST_CASE("Yolov5Detect") {
  Yolov5Detect *detect = new Yolov5Detect();
  detect->Init("");

  std::ifstream ifs("./828706969.rgb", std::ios::binary);
  std::vector<uint8_t> data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  std::vector<DetectBox> boxes = detect->Detect(data);
  for (const auto &box : boxes) {
    spdlog::info("{},{},{},{},{},{}\n", box.class_name, box.xmin, box.ymin, box.xmax, box.ymax, box.conf);

  }
}
