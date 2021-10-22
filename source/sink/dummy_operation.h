
#pragma once

#include "device_operation.h"

class DummyOperation : public ITouchScreenOperation {
public:
    bool Init() override { return true; }

    void Click(int x, int y) override {}

    void Move(int x_src, int y_src, int x_dst, int y_dst) override {}
};
