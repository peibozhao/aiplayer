
#pragma once

#include "common/log.h"
#include "device_operation.h"

class DummyOperation : public ITouchScreenOperation {
public:
    bool Init() override { return true; }

    void Click(uint16_t x, uint16_t y) override {
        LOG(INFO) << "Click " << x << " " << y;
    }

    void Move(int x_src, int y_src, int x_dst, int y_dst) override {
        LOG(INFO) << "Move (" << x_src << "," << y_src << ") -> (" << x_dst
                  << "," << y_dst << ")";
    }
};
