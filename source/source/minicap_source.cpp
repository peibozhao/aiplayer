
#include "minicap_source.h"
#include "common/log.h"
#include "common/util_functions.h"
#include "glog/logging.h"
#include <arpa/inet.h>
#include <stdexcept>
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

static bool ReadUtil(int fd, void *buffer, int buffer_size) {
    int read_len = 0;
    while (read_len < buffer_size) {
        int cur_read_len =
            read(fd, (char *)buffer + read_len, buffer_size - read_len);
        if (cur_read_len < 0) {
            LOG_INFO("minicap recv failed %d", errno);
            return false;
        } else if (cur_read_len == 0) {
            LOG_ERROR("Peer close");
            return false;
        }
        read_len += cur_read_len;
    }
    return true;
}

MinicapSource::MinicapSource(const std::string &ip, unsigned short port) {
    ip_ = ip;
    server_port_ = port;
}

MinicapSource::~MinicapSource() { close(socket_); }

bool MinicapSource::Init() {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        throw std::runtime_error("minicap socket create failed");
    }

    struct timeval overtime;
    memset(&overtime, 0, sizeof(overtime));
    overtime.tv_sec = 3;
    int setopt_ret = setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &overtime,
                                sizeof(overtime));
    if (setopt_ret != 0) {
        LOG_ERROR("setsockopt failed %d", setopt_ret);
        return false;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_.c_str());
    server_addr.sin_port = htons(server_port_);

    if (connect(socket_, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0) {
        throw std::runtime_error("minicap connect failed");
    }

    MinicapHeader header;
    if (!ReadUtil(socket_, &header, sizeof(header))) {
        throw std::runtime_error("minicap read head failed");
    }
    LOG_INFO("Minicap width %d height %d vwidth %d vheight %d orientate %d quirk %d",
             header.real_width, header.real_height, header.virtual_width,
             header.virtual_height, header.orientation, header.quirk);

    recv_thread_.reset(new std::thread(&MinicapSource::RecvImageThread, this));
    return true;
}

ImageFormat MinicapSource::GetFormat() { return ImageFormat::JPEG; }

std::vector<char> MinicapSource::GetImageBuffer() {
    std::lock_guard<std::mutex> lock(image_buffer_mutex_);
    return image_buffer_;
}

void MinicapSource::RecvImageThread() {
    while (true) {
        int frame_size;
        if (!ReadUtil(socket_, &frame_size, sizeof(frame_size))) {
            LOG_ERROR("Minicap get frame failed");
        }
        std::vector<char> tmp_image_buffer(frame_size);
        if (!ReadUtil(socket_, tmp_image_buffer.data(), tmp_image_buffer.size())) {
            LOG_ERROR("Minicap get frame failed");
        }

        std::lock_guard<std::mutex> lock(image_buffer_mutex_);
        image_buffer_ = tmp_image_buffer;
    }
}
