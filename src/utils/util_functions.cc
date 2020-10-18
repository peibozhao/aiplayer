
#include "util_functions.h"
#include <math.h>
#include <list>

float Sigmoid(float x) {
  return 1 / (1 + std::exp(-x));
}

std::vector<std::vector<float>> NMS(const std::vector<std::vector<float>> &boxes, float thresh) {
  std::vector<std::vector<float>> ret;
  std::list<std::vector<float>> boxes_list(boxes.begin(), boxes.end());
  while (!boxes_list.empty()) {
    auto max_iter = boxes_list.begin();
    for (auto iter = boxes_list.begin(); iter != boxes_list.end(); ++iter) {
      if ((*iter)[4] > (*max_iter)[4]) {
        max_iter = iter;
      }
    }
    ret.push_back(*max_iter);
    boxes_list.erase(max_iter);
    for (auto iter = boxes_list.begin(); iter != boxes_list.end();) {
      float iou = CalcIoU(ret[ret.size() - 1], *iter);
      if (iou > thresh) {
        iter = boxes_list.erase(iter);
      } else {
        ++iter;
      }
    }
  }
  return ret;
}

float CalcIoU(const std::vector<float> &lhs, const std::vector<float> &rhs) {
  float lhs_area = RectArea(lhs);
  float rhs_area = RectArea(rhs);
  float temp_area1 = RectArea({lhs[0], lhs[1], rhs[2], rhs[3]});
  float temp_area2 = RectArea({rhs[0], rhs[1], lhs[2], lhs[3]});
  float intersection_area = std::min(temp_area1, temp_area2);
  if (intersection_area < 0) {
    return 0;
  }
  float union_area = lhs_area + rhs_area - intersection_area;
  return intersection_area / union_area;
}

float RectArea(const std::vector<float> &rect) {
  return (rect[2] - rect[0]) * (rect[3] - rect[1]);
}

bool HandleSystemResult(int sys_ret) {
  return WIFEXITED(sys_ret) && (WEXITSTATUS(sys_ret) == 0);
}

