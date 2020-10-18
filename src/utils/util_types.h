
#ifndef UTILS_TYPES
#define UTILS_TYPES

#include <chrono>
#include "spdlog/spdlog.h"

class TimeLog {
public:
  TimeLog() {
    start_time_ = std::chrono::system_clock::now();
  }

  ~TimeLog() {
    auto end_time = std::chrono::system_clock::now();
    spdlog::info("Cost {}",
                 std::chrono::duration_cast<std::chrono::milliseconds>(
                     end_time - start_time_)
                     .count());
  }

private:
  std::chrono::system_clock::time_point start_time_;
};

#endif // ifndef UTILS_TYPES
