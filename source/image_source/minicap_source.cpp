
#include "minicap_source.h"
#include "common/util_functions.h"
#include "common/util_defines.h"
#include "glog/logging.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdexcept>

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

MinicapSource::MinicapSource(const std::string &ip, unsigned short port) {
    ip_ = ip;
    server_port_ = port;
}

MinicapSource::~MinicapSource() {
    close(socket_);
}

bool MinicapSource::Init() {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        throw std::runtime_error("minicap socket create failed");
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_.c_str());
    server_addr.sin_port = htons(server_port_);

    if (connect(socket_, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("minicap connect failed");
    }

    MinicapHeader header;
    if (!ReadUtil(socket_, &header, sizeof(header))) {
        throw std::runtime_error("minicap read head failed");
    }
    LOG_INFO("Minicap width %d height %d vwidth %d vheight %d quirk %d",
             header.real_width, header.real_height, header.virtual_width,
             header.virtual_height, header.quirk);
    return true;
}

ImageFormat MinicapSource::GetFormat() {
    return ImageFormat::JPEG;
}

std::vector<char> MinicapSource::GetImageBuffer() {
    std::vector<char> ret;
    int frame_size;
    if (!ReadUtil(socket_, &frame_size, sizeof(frame_size))) {
        LOG_ERROR("Minicap get frame failed");
        return ret;
    }
    ret.resize(frame_size);
    if (!ReadUtil(socket_, ret.data(), ret.size())) {
        LOG_ERROR("Minicap get frame failed");
        return ret;
    }
    return ret;
}
