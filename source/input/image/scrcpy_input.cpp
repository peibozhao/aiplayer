
#include "scrcpy_input.h"
#include "common/log.h"
#include "utils/util_functions.h"
#include <arpa/inet.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <string.h>
#include <sys/socket.h>

ScrcpyInput::ScrcpyInput(const std::string &ip, uint16_t port) {
  ip_ = ip;
  port_ = port;
}

ScrcpyInput::~ScrcpyInput() {
  is_running_ = false;
  recv_thread_->join();
};

bool ScrcpyInput::Init() {
  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_ < 0) {
    LOG(ERROR) << "Scrcpy socket create failed";
    return false;
  }

  struct timeval overtime;
  memset(&overtime, 0, sizeof(overtime));
  overtime.tv_sec = 3;
  int setopt_ret =
      setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &overtime, sizeof(overtime));
  if (setopt_ret != 0) {
    LOG(ERROR) << "setsockopt failed " << setopt_ret;
    return false;
  }

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip_.c_str());
  server_addr.sin_port = htons(port_);

  if (connect(socket_, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    LOG(ERROR) << "Scrcpy connect failed";
    return false;
  }

  is_running_ = true;
  recv_thread_.reset(new std::thread(&ScrcpyInput::RecvImageThread, this));
  return true;
}

cv::Mat ScrcpyInput::GetOneFrame() {
  std::unique_lock<std::mutex> lock(image_mutex_);
  image_con_.wait(lock, [this] { return !recv_buffer_.empty(); });

  int image_width = *(uint16_t *)(&recv_buffer_[0]),
      image_heigh = *(uint16_t *)(&recv_buffer_[2]);
  cv::Mat image(image_heigh * 3 / 2, image_width, CV_8UC1, recv_buffer_.data());
  cv::cvtColor(image, image, cv::COLOR_YUV2BGRA_I420);
  DLOG(INFO) << "Image shape " << image.cols << " " << image.rows;
  return image;
}

void ScrcpyInput::RecvImageThread() {
  while (is_running_) {
    uint32_t frame_size;
    if (!ReadUtil(socket_, &frame_size, sizeof(frame_size))) {
      LOG(ERROR) << "Scrcpy get frame failed. " << strerror(errno);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    frame_size = ntohl(frame_size);
    std::vector<uint8_t> tmp_image_buffer(frame_size);
    if (!ReadUtil(socket_, tmp_image_buffer.data(), tmp_image_buffer.size())) {
      LOG(ERROR) << "Scrcpy get frame failed. " << strerror(errno);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    std::lock_guard<std::mutex> lock(image_mutex_);
    recv_buffer_.swap(tmp_image_buffer);
    (*(uint16_t *)(&recv_buffer_[0])) = ntohs(*(uint16_t *)(&recv_buffer_[0]));
    (*(uint16_t *)(&recv_buffer_[2])) = ntohs(*(uint16_t *)(&recv_buffer_[2]));

    image_con_.notify_one();
  }
}
