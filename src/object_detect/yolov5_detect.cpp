
#include "yolov5_detect.h"
#include <memory>
#include <fstream>
#include "MNN/ImageProcess.hpp"
#include "utils/util_functions.h"
#include "yaml-cpp/yaml.h"
#include "spdlog/spdlog.h"
#include "utils/util_types.h"

Yolov5Detect::~Yolov5Detect() {
}

bool Yolov5Detect::Init(const std::string &cfg) {
  spdlog::info("Detect config: \n{}", cfg);
  std::string net_fn;
  try {
    YAML::Node config = YAML::Load(cfg);
    img_height_ = config["image_height"].as<int>();
    img_width_ = config["image_width"].as<int>();
    YAML::Node net_config = config["network"];
    input_height_ = net_config["input_height"].as<int>();
    input_width_ = net_config["input_width"].as<int>();
    class_names_ = net_config["class_names"].as<std::vector<std::string>>();
    nms_thresh_ = net_config["nms_thresh"].as<float>();
    score_thresh_ = net_config["score_thresh"].as<float>();
    YAML::Node stride_config = net_config["stride"];
    // for (const YAML::Node &stride_sub_config : stride_config) {
    for (const auto &stride_pair : net_config["stride"]) {
      YAML::Node stride_sub_config = stride_pair.second;
      stride_[stride_pair.first.as<std::string>()] = stride_sub_config.as<int>();
    }
    for (const auto &anchor_pair : net_config["anchor_grid"]) {
      std::string output_name = anchor_pair.first.as<std::string>();
      YAML::Node anchor_sub_config = anchor_pair.second;
      anchor_grid_[output_name] = anchor_sub_config.as<std::array<std::array<int, 2>, 3>>();
    }
    net_fn = net_config["net_file"].as<std::string>();
  } catch (std::exception &e) {
    spdlog::error("Catch error: {}", e.what());
    return false;
  }
  net_ = MNN::Interpreter::createFromFile(net_fn.c_str());

  MNN::ScheduleConfig sche_config;
  // sche_config.numThread = 1;
  MNN::BackendConfig backend_config;
  backend_config.precision = MNN::BackendConfig::Precision_Low;
  backend_config.memory = MNN::BackendConfig::Memory_Low;
  backend_config.power = MNN::BackendConfig::Power_Low;
  session_ = net_->createSession(sche_config);
  net_->releaseModel();
  return true;
}

bool Yolov5Detect::SetParam(const std::string &key, const std::string &value) {
  if (key == "image_width") {
    img_width_ = std::stoi(value);
  } else if (key == "image_height") {
    img_height_ = std::stoi(value);
  } else {
    return false;
  }
  return true;
}

std::vector<DetectBox> Yolov5Detect::Detect(const std::vector<uint8_t> &data) {
  TimeLog time_log("Detect");
  PreProcess(data);
  if (net_->runSession(session_) != MNN::NO_ERROR) {
    spdlog::error("MNN runSession failed");
    return {};
  }
  return PostProcess();
}

void Yolov5Detect::PreProcess(const std::vector<uint8_t> &data) {
  MNN::CV::ImageProcess::Config cv_config;
  cv_config.sourceFormat = MNN::CV::RGB;
  cv_config.destFormat = MNN::CV::RGB;
  cv_config.filterType = MNN::CV::NEAREST;
  cv_config.normal[0] = 1.f / 255.f;
  cv_config.normal[1] = 1.f / 255.f;
  cv_config.normal[2] = 1.f / 255.f;
  std::shared_ptr<MNN::CV::ImageProcess> cv_proc(MNN::CV::ImageProcess::create(cv_config));
  MNN::CV::Matrix trans_matrix;
  trans_matrix.postScale(float(input_width_) / img_width_, float(input_height_) / img_height_);
  trans_matrix.invert(&trans_matrix);
  cv_proc->setMatrix(trans_matrix);
  cv_proc->convert(data.data(), img_width_, img_height_, 0, net_->getSessionInput(session_, nullptr));
}

std::vector<DetectBox> Yolov5Detect::PostProcess() {
  std::vector<std::vector<float>> boxes;
  std::map<std::string, MNN::Tensor*> outputs = net_->getSessionOutputAll(session_);
  int one_pred_num = class_names_.size() + 5;
  for (auto output_pair : outputs) {
    MNN::Tensor *output = output_pair.second;
    int stride = stride_[output_pair.first];
    const std::array<std::array<int, 2>, 3> &anchor_grid =
        anchor_grid_[output_pair.first];
    for (int batch = 0; batch < output->batch(); ++batch) {
      for (int channel = 0; channel < output->channel(); ++channel) {
        for (int height = 0; height < output->height(); ++height) {
          for (int width = 0; width < output->width(); ++width) {
            int offset = batch * output->channel() * output->height() *
                             output->width() * one_pred_num +
                         channel * output->height() * output->width() * one_pred_num +
                         height * output->width() * one_pred_num + width * one_pred_num;
            float *cur_box_ptr = output->host<float>() + offset;
            for (int i = 0; i < one_pred_num; ++i) {
              cur_box_ptr[i] = Sigmoid(cur_box_ptr[i]);
            }

            float x = (cur_box_ptr[0] * 2 - 0.5 + width) * stride;
            float y = (cur_box_ptr[1] * 2 - 0.5 + height) * stride;
            float w = std::pow(cur_box_ptr[2] * 2, 2) * anchor_grid[channel][0];
            float h = std::pow(cur_box_ptr[3] * 2, 2) * anchor_grid[channel][1];

            float *max_ele = std::max_element(cur_box_ptr + 5, cur_box_ptr + one_pred_num);
            float conf = cur_box_ptr[4] * *max_ele;
            if (conf > score_thresh_) {
              std::vector<float> box{x - w / 2, y - h / 2,
                                     x + w / 2, y + h / 2,
                                     conf,      float(max_ele - cur_box_ptr - 5)};
              boxes.emplace_back(std::move(box));
            }
          }
        }
      }
    }
  }
  std::vector<DetectBox> ret;
  std::vector<std::vector<float>> nms_boxes = NMS(boxes, nms_thresh_);
  for (const std::vector<float> &nms_box : nms_boxes) {
    DetectBox box;
    box.xmin = nms_box[0] * img_width_ / input_width_;
    box.ymin = nms_box[1] * img_height_ / input_height_;
    box.xmax = nms_box[2] * img_width_ / input_width_;
    box.ymax = nms_box[3] * img_height_ / input_height_;
    box.conf = nms_box[4];
    box.class_name = class_names_[static_cast<int>(nms_box[5])];
    ret.emplace_back(std::move(box));
  }
  return ret;
}
