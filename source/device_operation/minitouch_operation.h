
#pragma once

#include "touch_screen_operation.h"

class MinitouchOperation : public ITouchScreenOperation {
public:
    MinitouchOperation(unsigned short port);

    ~MinitouchOperation() override;

    bool Init() override;

    bool Click(int x, int y) override;

    bool Move(int x_src, int y_src, int x_dst, int y_dst) override;

private:
    unsigned short server_port_;

    int socket_;
};
