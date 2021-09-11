
#include "minitouch_operation.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdexcept>
#include <unistd.h>
#include "common/util_defines.h"
#include "httplib.h"

MinitouchOperation::MinitouchOperation(unsigned short port) {
    server_port_ = port;
}

MinitouchOperation::~MinitouchOperation() {
    close(socket_);
}

bool MinitouchOperation::Init() {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        throw std::runtime_error("minitouch create socket failed");
    }

    sockaddr_in peer_sock;
    memset(&peer_sock, 0, sizeof(peer_sock));
    peer_sock.sin_family = AF_INET;
    peer_sock.sin_addr.s_addr = inet_addr("127.0.0.1");
    peer_sock.sin_port = htons(server_port_);
    if (connect(socket_, (sockaddr *)&peer_sock, sizeof(peer_sock)) < 0) {
        throw std::runtime_error("minitouch connect failed");
    }

    // Must read
    char buf[1000];
    int read_len = read(socket_, buf, sizeof(buf));
    LOG_INFO("%s", buf);
    return true;
}

bool MinitouchOperation::Click(int x, int y) {
    std::stringstream ss;
    ss << "d 0 " << std::to_string(x) << ' ' << std::to_string(y) << " 0\n";
    ss << "c\n";
    ss << "u 0\n";
    ss << "c\n";
    std::string data = ss.str();
    int write_len = write(socket_, data.c_str(), data.size());
    if (write_len != data.size()) {
        LOG_ERROR("write len error %ld %d", data.size(), write_len);
    }
    return true;
}

bool MinitouchOperation::Move(int x_src, int y_src, int x_dst, int y_dst) {
    return false;
}
