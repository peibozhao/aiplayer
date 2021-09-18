
#pragma once

#include "device_operation.h"
#include <string>

class MinitouchOperation : public ITouchScreenOperation {
public:
    MinitouchOperation(const std::string &ip, unsigned short port);

    ~MinitouchOperation() override;

    bool Init() override;

    int TouchDown(int x, int y) override;

    void Move(int id, int x_dst, int y_dst) override;

    void TouchUp(int id) override;

private:
    std::string ip_;
    unsigned short server_port_;

    int socket_;
};
