
#pragma once

class ITouchScreenOperation {
public:
    virtual ~ITouchScreenOperation() {}

    virtual bool Init() { return true; }

    virtual int TouchDown(int x, int y) = 0;

    virtual void Move(int id, int x_dst, int y_dst) = 0;

    virtual void TouchUp(int id) = 0;
};
