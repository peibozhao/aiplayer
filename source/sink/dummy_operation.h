
#pragma once

#include "device_operation.h"
#include "common/log.h"

class DummyOperation : public ITouchScreenOperation {
public:
    bool Init() override { return true; }

    void Click(int x, int y) override {
        LOG_INFO("Click %d %d", x, y);
    }

    void Move(int x_src, int y_src, int x_dst, int y_dst) override {
        LOG_INFO("Move (%d, %d) -> (%d, %d)", x_src, y_src, x_dst, y_dst);
    }
};
