/**
 * @file yolov5_detect.h
 * @brief 基于yolov5的目标检测
 * @author peibozhao
 * @version 1.0.0
 * @date 2020-10-11
 */

#ifndef OBJECT_DETECT_YOLOV5_DETECT_H
#define OBJECT_DETECT_YOLOV5_DETECT_H

#include <map>
#include "MNN/Interpreter.hpp"
#include "detect.h"

/**
 * @brief 基于 https://github.com/ultralytics/yolov5 pytorch->onnx->mnn
 *    input:
 *      images: 1*3*320*640
 *    output:
 *      output: 1*3*40*80*6
 *      1036: 1*3*20*40*6
 *      1056: 1*3*10*20*6
 */
class Yolov5Detect : public IDetect {
public:
  ~Yolov5Detect() override;
  bool Init(const std::string &cfg) override;
  bool SetParam(const std::string &key, const std::string &value) override;
  std::vector<DetectBox> Detect(const std::vector<uint8_t> &data) override;

private:
  void PreProcess(const std::vector<uint8_t> &data);
  std::vector<DetectBox> PostProcess();

private:
  // 配置
  int img_height_, img_width_;
  float score_thresh_;
  float nms_thresh_;
  std::vector<std::string> class_names_;
  // 网络
  int input_height_, input_width_;
  std::map<std::string, int> stride_;
  std::map<std::string, std::array<std::array<int, 2>, 3>> anchor_grid_;
  MNN::Interpreter *net_;
  MNN::Session *session_;
};

#endif // ifndef OBJECT_DETECT_YOLOV5_DETECT_H

