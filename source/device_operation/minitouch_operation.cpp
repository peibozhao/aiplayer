
#include "minitouch_operation.h"
#include "common/util_defines.h"
#include "httplib.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

MinitouchOperation::MinitouchOperation(const std::string &ip, unsigned short port) {
    ip_ = ip;
    server_port_ = port;
}

MinitouchOperation::~MinitouchOperation() { close(socket_); }

bool MinitouchOperation::Init() {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        throw std::runtime_error("minitouch create socket failed");
    }

    sockaddr_in peer_sock;
    memset(&peer_sock, 0, sizeof(peer_sock));
    peer_sock.sin_family = AF_INET;
    peer_sock.sin_addr.s_addr = inet_addr(ip_.c_str());
    peer_sock.sin_port = htons(server_port_);
    if (connect(socket_, (sockaddr *)&peer_sock, sizeof(peer_sock)) < 0) {
        throw std::runtime_error("minitouch connect failed");
    }

    // Must read
    char buf[1000];
    int return_count = 0;
    int read_len = 0;
    while (return_count < 3) {
        read_len += read(socket_, buf, sizeof(buf));
        for (int i = 0; i < read_len; ++i) {
            if (buf[i] == '\n')
                ++return_count;
        }
    }
    LOG_INFO("%s", buf);
    return true;
}

int MinitouchOperation::TouchDown(int x, int y) {
    std::stringstream op_ss;
    op_ss << "d 0 " << std::to_string(x) << ' ' << std::to_string(y) << " 0\n";
    op_ss << "c\n";
    std::string op_str = op_ss.str();
    int write_len = write(socket_, op_str.c_str(), op_str.size());
    if (write_len != op_str.size()) {
        LOG_ERROR("write len error %ld %d", op_str.size(), write_len);
    }
    return true;
}

void MinitouchOperation::Move(int id, int x_dst, int y_dst) {
    std::stringstream op_ss;
    op_ss << "m " << std::to_string(id) << ' '
          << std::to_string(x_dst) << ' ' << std::to_string(y_dst) << " 0\n";
    op_ss << "c\n";
    std::string op_str = op_ss.str();
    int write_len = write(socket_, op_str.c_str(), op_str.size());
    if (write_len != op_str.size()) {
        LOG_ERROR("write len error %ld %d", op_str.size(), write_len);
    }
}

void MinitouchOperation::TouchUp(int id) {
    std::stringstream op_ss;
    op_ss << "u 0\n";
    op_ss << "c\n";
    std::string op_str = op_ss.str();
    int write_len = write(socket_, op_str.c_str(), op_str.size());
    if (write_len != op_str.size()) {
        LOG_ERROR("write len error %ld %d", op_str.size(), write_len);
    }
}
