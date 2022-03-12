#pragma once

#include "device_operation.h"
#include <mutex>
#include <string>

class ScrcpyOperation : public ITouchScreenOperation {
public:
  ScrcpyOperation(const std::string &ip, uint16_t port);

  ~ScrcpyOperation() override;

  bool Init() override;

  void Click(uint16_t x, uint16_t y) override;

private:
  void SendPressDown(uint16_t x, uint16_t y);

  void SendPressUp(uint16_t x, uint16_t y);

private:
  std::string ip_;
  uint16_t server_port_;
  int socket_;
  std::mutex mutex_;
};
