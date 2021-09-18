
#pragma once

class IApplication {
public:
    virtual bool Init() { return true; }

    virtual void Run() = 0;

    virtual void Stop() = 0;
};
