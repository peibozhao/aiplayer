
#include "scrcpy_operation.h"
#include "utils/log.h"
#include "httplib.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#pragma pack(1)
struct PressMessage {
    uint8_t touch_flag;
    uint8_t is_up;
    uint64_t pointer_id;
    uint32_t x;
    uint32_t y;
    uint16_t frame_size_x;
    uint16_t frame_size_y;
    uint16_t pressure;
    uint32_t buttons;
};
#pragma pack()

static int SendPress(int socket, const PressMessage &message) {
    uint32_t buffer_size = sizeof(PressMessage) + 4;
    uint8_t *buffer = (uint8_t *)malloc(buffer_size);
    memset(buffer, 0, buffer_size);
    *((uint32_t *)&buffer[0]) = htonl(buffer_size - 4);
    buffer[4 + 0] = message.touch_flag;
    buffer[4 + 1] = message.is_up;
    *((uint32_t *)&buffer[4 + 10]) = htonl(message.x);
    *((uint32_t *)&buffer[4 + 14]) = htonl(message.y);
    int send_len = send(socket, buffer, buffer_size, 0);
    if (send_len != buffer_size) {
        LOG_ERROR("Scrcpy send failed. %d != %d", send_len, buffer_size);
    }
    return send_len;
}

ScrcpyOperation::ScrcpyOperation(const std::string &ip, unsigned short port) {
    ip_ = ip;
    server_port_ = port;
    socket_ = -1;
}

ScrcpyOperation::~ScrcpyOperation() { close(socket_); }

bool ScrcpyOperation::Init() {
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0) {
        LOG_ERROR("Scrcpy create socket failed");
        return false;
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
        LOG_ERROR("Scrcpy connect failed");
        return false;
    }
    return true;
}

void ScrcpyOperation::Click(uint16_t x, uint16_t y) {
    SendPressDown(x, y);
    SendPressUp(x, y);
}

void ScrcpyOperation::SendPressDown(uint16_t x, uint16_t y) {
    PressMessage press_down_msg;
    press_down_msg.touch_flag = 2;
    press_down_msg.is_up = 0;
    // pointer_id
    press_down_msg.x = x;
    press_down_msg.y = y;
    // frame size x
    // frame size y
    // pressure
    // buttons

    int send_len = SendPress(socket_, press_down_msg);
}

void ScrcpyOperation::SendPressUp(uint16_t x, uint16_t y) {
    PressMessage press_down_msg;
    press_down_msg.touch_flag = 2;
    press_down_msg.is_up = 1;
    // pointer_id
    press_down_msg.x = x;
    press_down_msg.y = y;
    // frame size x
    // frame size y
    // pressure
    // buttons

    int send_len = SendPress(socket_, press_down_msg);
}
