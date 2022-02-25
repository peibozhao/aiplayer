
#pragma once

#include "glog/logging.h"
#include <chrono>
#include <fstream>
#include <stdio.h>

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
        DLOG(INFO) << label_ << " cost "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(
                           end_time - start_time_)
                           .count();
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
