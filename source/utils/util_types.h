
#pragma once

#include "spdlog/spdlog.h"
#include <chrono>

class TimeLog {
public:
    TimeLog(const std::string &label = "") {
        label_ = label;
        start_time_ = std::chrono::system_clock::now();
    }

    ~TimeLog() {
        auto end_time = std::chrono::system_clock::now();
        SPDLOG_DEBUG(
            "{} cost {}", label_,
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_).count());
    }

private:
    std::chrono::system_clock::time_point start_time_;
    std::string label_;
};
