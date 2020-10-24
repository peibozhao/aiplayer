
#include "util_converter.h"

DetectObject Converter(const DetectBox &box) {
  DetectObject ret;
  ret.conf = box.conf;
  ret.name = box.class_name;
  ret.xmax = box.xmax;
  ret.xmin = box.xmin;
  ret.ymax = box.ymax;
  ret.ymin = box.ymin;
  return ret;
}

std::vector<DetectObject> Converter(const std::vector<DetectBox> &boxs) {
  std::vector<DetectObject> ret;
  for (auto &box : boxs) {
    ret.emplace_back(Converter(box));
  }
  return ret;
}

DetectObject Converter(const DetectWord &word) {
  DetectObject ret;
  ret.conf = word.conf;
  ret.name = word.word;
  ret.xmax = word.xmax;
  ret.xmin = word.xmin;
  ret.ymax = word.ymax;
  ret.ymin = word.ymin;
  return ret;
}

std::vector<DetectObject> Converter(const std::vector<DetectWord> &words) {
  std::vector<DetectObject> ret;
  for (auto &word : words) {
    ret.emplace_back(Converter(word));
  }
  return ret;
}
