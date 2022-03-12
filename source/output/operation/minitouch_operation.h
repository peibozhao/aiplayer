/**
 * @file minitouch_operation.h
 * @brief Operator by minitouch
 * @note Screen must be portrait at begin
 */

#pragma once

#include "device_operation.h"
#include <mutex>
#include <string>

class MinitouchOperation : public ITouchScreenOperation {
public:
  MinitouchOperation(const std::string &ip, uint16_t port, uint16_t width,
                     uint16_t height, int orientation);

  ~MinitouchOperation() override;

  bool Init() override;

  void Click(uint16_t x, uint16_t y) override;

private:
  void ParseHeader(char *buffer, int len);

  std::pair<uint16_t, uint16_t> CoordinateConvertion(uint16_t x, uint16_t y);

private:
  std::string ip_;
  uint16_t server_port_;
  int socket_;
  std::mutex mutex_;

  uint16_t image_width_, image_height_;
  int orientation_;
  uint16_t max_x_, max_y_;
};
