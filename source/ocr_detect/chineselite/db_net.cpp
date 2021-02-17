
#include "db_net.h"
#include "MNN/ImageProcess.hpp"
#include <algorithm>

bool DbNet::InitModel(const std::string &model_fname) {
  mnn_net_.reset(MNN::Interpreter::createFromFile(model_fname.c_str()));
  if (mnn_net_ == nullptr) {
    return false;
  }
  MNN::ScheduleConfig mnn_config;
  mnn_session_ = mnn_net_->createSession(mnn_config);
  return mnn_session_ != nullptr;
}

std::vector<Box> DbNet::Detect(const cv::Mat &image) {
  MNN::Tensor *mnn_input = mnn_net_->getSessionInput(mnn_session_, nullptr);
  std::vector<int> input_shape{1, 3, image.rows, image.cols};
  mnn_net_->resizeTensor(mnn_input, input_shape);
  mnn_net_->resizeSession(mnn_session_);

  auto batch = mnn_input->batch();
  auto channel = mnn_input->channel();
  auto height = mnn_input->height();
  auto width = mnn_input->width();

  std::shared_ptr<MNN::Tensor> nchw_input(
      MNN::Tensor::create<float>(input_shape, nullptr, MNN::Tensor::CAFFE));

  for (int ch = 0; ch < nchw_input->channel(); ++ch) {
    for (int h = 0; h < nchw_input->height(); ++h) {
      for (int w = 0; w < nchw_input->width(); ++w) {
        nchw_input->host<float>()[ch * height * width + h * width + w] =
            (image.data[h * image.cols * image.channels() + w * image.channels() + ch] -
             mean_[ch]) *
            norm_[ch];
      }
    }
  }
  mnn_input->copyFromHostTensor(nchw_input.get());

  MNN::ErrorCode predict_ret = mnn_net_->runSession(mnn_session_);
  if (predict_ret != MNN::NO_ERROR) {
    std::cout << "Predict Error" << std::endl;
    return {};
  }

  std::vector<int> output_shape{1, 1, image.rows, image.cols};
  MNN::Tensor *mnn_output = mnn_net_->getSessionOutput(mnn_session_, nullptr);
  MNN::Tensor *nchw_output =
      MNN::Tensor::create<float>(output_shape, NULL, MNN::Tensor::TENSORFLOW);
  mnn_output->copyToHostTensor(nchw_output);

  cv::Mat output_image(image.rows, image.cols, CV_32FC1);
  memcpy(output_image.data, nchw_output->host<float>(), image.cols * image.rows * sizeof(float));
  cv::Mat norm_image = output_image >= thresh_;
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(norm_image, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
  std::vector<Box> boxes_ret;
  for (const std::vector<cv::Point> &contour : contours) {
    cv::Rect rect = cv::boundingRect(contour);
    if (rect.area() < min_area_) {
      continue;
    }

    float unclip_offset = unclip_ratio_ * rect.width * rect.height / (2 * (rect.width + rect.height));
    Box box;
    box.x = std::max(rect.x - unclip_offset, 0.f);
    box.y = std::max(rect.y - unclip_offset, 0.f);
    box.width = std::min(rect.width + unclip_offset * 2, image.cols - box.x);
    box.height = std::min(rect.height + unclip_offset * 2, image.rows - box.y);
    boxes_ret.push_back(box);
  }
  return boxes_ret;
}
