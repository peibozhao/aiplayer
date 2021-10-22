
#include "minitouch_operation.h"
#include "common/log.h"
#include "httplib.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

MinitouchOperation::MinitouchOperation(const std::string &ip,
                                       unsigned short port,
                                       const ImageInfo &image_info) {
    ip_ = ip;
    server_port_ = port;
    socket_ = -1;
    width_ = image_info.width;
    height_ = image_info.height;
    orientation_ = image_info.orientation;
}

MinitouchOperation::~MinitouchOperation() { close(socket_); }

bool MinitouchOperation::Init() {
    if (orientation_ != 0 && orientation_ != 90) {
        throw std::runtime_error("Ensupport orientation value " +
                                 std::to_string(orientation_));
    }

    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        throw std::runtime_error("minitouch create socket failed");
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

    sockaddr_in peer_sock;
    memset(&peer_sock, 0, sizeof(peer_sock));
    peer_sock.sin_family = AF_INET;
    peer_sock.sin_addr.s_addr = inet_addr(ip_.c_str());
    peer_sock.sin_port = htons(server_port_);
    if (connect(socket_, (sockaddr *)&peer_sock, sizeof(peer_sock)) < 0) {
        throw std::runtime_error("minitouch connect failed");
    }

    // If dont recv header, need to restart minitouch
    char buf[1000] = "\0";
    int return_count = 0;
    while (return_count < 3) {
        int read_len = read(socket_, buf, sizeof(buf));
        if (read_len == -1) {
            LOG_ERROR("Network error %d", errno);
            return false;
        } else if (read_len == 0) {
            LOG_ERROR("Peer closed");
            return false;
        }
        for (int i = 0; i < read_len; ++i) {
            if (buf[i] == '\n')
                ++return_count;
        }
    }

    if (buf[0] != '\0')
        LOG_INFO("%s", buf)
    else
        LOG_INFO("Minitouch has no header");
    return true;
}

void MinitouchOperation::Click(int x, int y) {
    std::pair<int, int> point = TurnAroundPoint(x, y);
    std::tie(x, y) = point;
    LOG_INFO("Click %d %d", x, y);

    std::stringstream op_ss;
    op_ss << "d 0 " << std::to_string(x) << ' ' << std::to_string(y) << " 1\n"
          << "c\n" << "u 0\n" << "c\n";
    std::string op_str = op_ss.str();
    int write_len = write(socket_, op_str.c_str(), op_str.size());
    if (write_len != op_str.size()) {
        LOG_ERROR("write len error %ld %d", op_str.size(), write_len);
    }
}

std::pair<int, int> MinitouchOperation::TurnAroundPoint(int x, int y) {
    if (orientation_ == 0) {
        return std::make_pair(x, y);
    } else if (orientation_ == 90) {
        return std::make_pair(height_ - y, x);
    }
    return std::make_pair(0, 0);
}

