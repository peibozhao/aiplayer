
#include "minitouch_operation.h"
#include "common/log.h"
#include "httplib.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

MinitouchOperation::MinitouchOperation(const std::string &ip, unsigned short port,
                                       const ImageInfo &image_info, int orientation) {
    ip_ = ip;
    server_port_ = port;
    socket_ = -1;
    image_width_ = image_info.width;
    image_height_ = image_info.height;
    orientation_ = orientation;
    max_x_ = -1, max_y_ = -1;
}

MinitouchOperation::~MinitouchOperation() { close(socket_); }

bool MinitouchOperation::Init() {
    if (orientation_ != 0 && orientation_ != 90) {
        LOG_ERROR("Ensupport orientation value %d", orientation_);
        return false;
    }

    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        LOG_ERROR("Minitouch create socket failed");
        return false;
    }

    struct timeval overtime;
    memset(&overtime, 0, sizeof(overtime));
    overtime.tv_sec = 3;
    int setopt_ret = setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &overtime, sizeof(overtime));
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
        LOG_ERROR("Minitouch connect failed");
        return false;
    }

    // If dont recv header, need to restart minitouch
    char header_buf[100] = "\0";
    int header_len = 0;
    int return_count = 0;
    while (return_count < 3) {
        int read_len = read(socket_, header_buf + header_len, sizeof(header_buf) - header_len);
        if (read_len == -1) {
            LOG_ERROR("Network error %d", errno);
            return false;
        } else if (read_len == 0) {
            LOG_ERROR("Peer closed");
            return false;
        }
        for (int i = 0; i < read_len; ++i) {
            if (header_buf[header_len + i] == '\n') ++return_count;
        }
        header_len += read_len;
    }

    LOG_INFO("%s", header_buf);
    ParseHeader(header_buf, header_len);
    return true;
}

void MinitouchOperation::Click(int x, int y) {
    std::pair<int, int> point = CoordinateConvertion(x, y);
    std::tie(x, y) = point;
    LOG_INFO("Click %d %d", x, y);

    std::stringstream op_ss;
    op_ss << "d 0 " << std::to_string(x) << ' ' << std::to_string(y) << " 1\n"
          << "c\n"
          << "u 0\n"
          << "c\n";
    std::string op_str = op_ss.str();
    std::lock_guard<std::mutex> lock(mutex_);
    int write_len = write(socket_, op_str.c_str(), op_str.size());
    if (write_len != op_str.size()) {
        LOG_ERROR("write len error %ld %d", op_str.size(), write_len);
        return;
    }
}

void MinitouchOperation::ParseHeader(char *buffer, int len) {
    buffer[len] = '\0';
    sscanf(buffer, "v %*d\n^ %*d %d %d %*d\n$ %*d\n", &max_x_, &max_y_);
    LOG_INFO("Max x %d, max y %d", max_x_, max_y_);
}

std::pair<int, int> MinitouchOperation::CoordinateConvertion(int x, int y) {
    int ret_x = 0, ret_y = 0;
    if (orientation_ == 0) {
        ret_x = x * max_x_ / image_width_;
        ret_y = y * max_y_ / image_height_;
    } else if (orientation_ == 90) {
        ret_x = max_x_ - y * max_x_ / image_height_;
        ret_y = x * max_y_ / image_width_;
    }
    return std::make_pair(ret_x, ret_y);
}
