
#pragma once

#include "image_source.h"
#include <string>
#include <thread>
#include <memory>
#include <mutex>

class MinicapSource : public IImageSource {
public:
    MinicapSource(const std::string &ip, unsigned short port);

    ~MinicapSource() override;

    bool Init() override;

    ImageFormat GetFormat() override;

    std::vector<char> GetImageBuffer() override;

private:
    void RecvImageThread();

private:
    std::string ip_;
    unsigned short server_port_;
    int socket_;

    std::shared_ptr<std::thread> recv_thread_;
    std::mutex image_buffer_mutex_;
    std::vector<char> image_buffer_;
};

