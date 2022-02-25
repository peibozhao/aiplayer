
#include "minicap_input.h"
#include "common/log.h"
#include "utils/util_functions.h"
#include <arpa/inet.h>
#include <opencv2/imgcodecs.hpp>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#pragma pack(1)
struct MinicapHeader {
    uint8_t version;
    uint8_t header_size;
    uint32_t pid;
    uint32_t real_width;
    uint32_t real_height;
    uint32_t virtual_width;
    uint32_t virtual_height;
    uint8_t orientation;
    uint8_t quirk;
};

struct MinicapFrame {
    uint32_t size;
    unsigned char buffer[0];
};
#pragma pack()

MinicapInput::MinicapInput(const std::string &ip, unsigned short port) {
    ip_ = ip;
    port_ = port;
    is_running_ = false;
}

MinicapInput::~MinicapInput() {
    is_running_ = false;
    recv_thread_->join();
    close(socket_);
}

bool MinicapInput::Init() {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        LOG(ERROR) << "Minicap socket create failed";
        return false;
    }

    struct timeval overtime;
    memset(&overtime, 0, sizeof(overtime));
    overtime.tv_sec = 3;
    int setopt_ret = setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &overtime,
                                sizeof(overtime));
    if (setopt_ret != 0) {
        LOG(ERROR) << "setsockopt failed " << setopt_ret;
        return false;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_.c_str());
    server_addr.sin_port = htons(port_);

    if (connect(socket_, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0) {
        LOG(ERROR) << "Minicap connect failed";
        return false;
    }

    MinicapHeader header;
    if (!ReadUtil(socket_, &header, sizeof(header))) {
        LOG(ERROR) << "Minicap read head failed";
        return false;
    }
    LOG(INFO) << "Minicap header " << header.real_width << header.real_height
              << header.virtual_width << header.virtual_height
              << header.orientation << header.quirk;

    // if (header.orientation == 0 || header.orientation == 2) {
    //     header.virtual_width;
    //     header.virtual_height;
    // } else if (header.orientation == 1 || header.orientation == 3) {
    //     header.virtual_height;
    //     header.virtual_width;
    // } else {
    //     LOG_ERROR("Minicap unsopport orientation %d", header.orientation);
    //     return false;
    // }
    //
    is_running_ = true;
    recv_thread_.reset(new std::thread(&MinicapInput::RecvImageThread, this));

    return true;
}

cv::Mat MinicapInput::GetOneFrame() {
    std::unique_lock<std::mutex> lock(image_mutex_);
    image_con_.wait(lock, [this] { return !recv_buffer_.empty(); });
    return cv::imdecode(recv_buffer_, cv::ImreadModes::IMREAD_COLOR);
}

void MinicapInput::RecvImageThread() {
    while (is_running_) {
        uint32_t frame_size;
        // Byte order
        if (!ReadUtil(socket_, &frame_size, sizeof(frame_size))) {
            LOG(ERROR) << "Minicap get frame failed";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        std::vector<uint8_t> tmp_image_buffer(frame_size);
        if (!ReadUtil(socket_, tmp_image_buffer.data(),
                      tmp_image_buffer.size())) {
            LOG(ERROR) << "Minicap get frame failed";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        std::lock_guard<std::mutex> lock(image_mutex_);
        recv_buffer_.swap(tmp_image_buffer);
        image_con_.notify_one();
    }
}
