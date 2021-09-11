
#pragma once

class ITouchScreenOperation {
public:
    virtual ~ITouchScreenOperation() {}

    virtual bool Init() { return true; }

    virtual bool Click(int x, int y) = 0;

    virtual bool Move(int x_src, int y_src, int x_dst, int y_dst) = 0;
};
