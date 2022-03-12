
#pragma once

#include <stdexcept>

class ITouchScreenOperation {
public:
  virtual ~ITouchScreenOperation() {}

  virtual bool Init() { return true; }

  virtual void Click(uint16_t x, uint16_t y) {
    throw std::runtime_error("Unsupport click");
  }

  virtual void Move(int x_src, int y_src, int x_dst, int y_dst) {
    throw std::runtime_error("Unsupport move");
  }
};
