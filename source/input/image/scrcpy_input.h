
#pragma once

#include "image_input.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ScrcpyInput : public IImageInput {
public:
    ScrcpyInput(const std::string &ip, uint16_t port);

    ~ScrcpyInput() override;

    bool Init() override;

    cv::Mat GetOneFrame() override;

private:
    void RecvImageThread();

private:
    std::string ip_;
    uint16_t port_;
    int socket_;

    std::shared_ptr<std::thread> recv_thread_;
    std::mutex image_mutex_;
    std::condition_variable image_con_;
    std::vector<uint8_t> recv_buffer_;  // YUV420p

    std::atomic_bool is_running_;
};
