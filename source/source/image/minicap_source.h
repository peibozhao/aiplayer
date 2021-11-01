
#pragma once

#include "source.h"
#include <string>
#include <thread>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>

class MinicapSource : public ISource {
public:
    MinicapSource(const std::string &ip, unsigned short port);

    ~MinicapSource() override;

    bool Init() override;

    void Start() override;

    void Stop() override;

    ImageInfo GetImageInfo() override;

    std::vector<char> GetImageBuffer() override;

private:
    void RecvImageThread();

private:
    std::string ip_;
    unsigned short server_port_;
    int socket_;

    std::shared_ptr<std::thread> recv_thread_;
    std::mutex image_mutex_;
    std::condition_variable image_con_;
    std::vector<char> image_buffer_;
    ImageInfo image_info_;

    std::atomic_bool is_running_;
};

