
#pragma once

#include "touch_screen_operation.h"

class MinitouchOperation : public ITouchScreenOperation {
public:
    MinitouchOperation(unsigned short port);

    ~MinitouchOperation() override;

    bool Init() override;

    int TouchDown(int x, int y) override;

    void Move(int id, int x_dst, int y_dst) override;

    void TouchUp(int id) override;

private:
    unsigned short server_port_;

    int socket_;
};
