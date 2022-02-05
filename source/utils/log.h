
#pragma once

#include "glog/logging.h"
#include <chrono>
#include <fstream>
#include <stdio.h>

#define LOG_TYPE(type, format, ...)                                            \
    {                                                                          \
        char string_log[1024] = {0};                                           \
        sprintf(string_log, format, ##__VA_ARGS__);                            \
        LOG(type) << string_log;                                               \
    }

#define LOG_DEBUG(format, ...)                                                 \
    {                                                                          \
        char string_log[10240] = {0};                                          \
        sprintf(string_log, format, ##__VA_ARGS__);                            \
        DLOG(INFO) << string_log;                                              \
    }

#define LOG_INFO(format, ...) LOG_TYPE(INFO, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) LOG_TYPE(ERROR, format, ##__VA_ARGS__)

/// Print cost time
class TimeLog {
public:
    TimeLog(const std::string &label = "") {
        label_ = label;
        printed_ = false;
        start_time_ = std::chrono::system_clock::now();
    }

    ~TimeLog() {
        if (!printed_) {
            Tok();
        }
    }

    void Tok() {
        auto end_time = std::chrono::system_clock::now();
        LOG_DEBUG("%s cost %ld", label_.c_str(),
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      end_time - start_time_)
                      .count());
        printed_ = true;
    }

private:
    std::chrono::system_clock::time_point start_time_;
    std::string label_;
    bool printed_;
};

/// Save buffer as file
inline void FileSaver(const std::vector<char> &buffer,
                      const std::string &fname) {
    std::ofstream ofs(fname, std::ios::binary);
    ofs.write(buffer.data(), buffer.size());
}

