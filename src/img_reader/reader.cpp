
#include "reader.h"

#include "opencv2/imgproc.hpp"

Image CvImageToImage(const cv::Mat &cv_img) {
  cv::Mat cv_img_rgb;
  cv::cvtColor(cv_img, cv_img_rgb, cv::COLOR_BGR2RGB);
  Image ret;
  ret.width = cv_img_rgb.cols;
  ret.height = cv_img_rgb.rows;
  ret.data.resize(ret.width * ret.height * 3);
  memcpy(ret.data.data(), cv_img_rgb.data, ret.data.size());
  return ret;
}

cv::Mat ImageToCvImage(const Image &img) {
  cv::Mat cv_img;
  cv_img.create(img.height, img.width, CV_8UC3);
  memcpy(cv_img.data, img.data.data(), img.data.size());
  cv::cvtColor(cv_img, cv_img, cv::COLOR_RGB2BGR);
  return cv_img;
}
