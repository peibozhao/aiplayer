#pragma once

#include "image_input.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class MinicapInput : public IImageInput {
public:
    MinicapInput(const std::string &ip, unsigned short port);

    ~MinicapInput() override;

    bool Init() override;

    Image GetOneFrame() override;

private:
    void RecvImageThread();

private:
    std::string ip_;
    unsigned short port_;
    int socket_;

    std::shared_ptr<std::thread> recv_thread_;
    std::mutex image_mutex_;
    std::condition_variable image_con_;
    std::vector<uint8_t> image_buffer_;

    std::atomic_bool is_running_;
};
