
#pragma once

class IApplication {
public:
    virtual bool Init() { return true; }

    virtual void Run() = 0;

    virtual void Pause() = 0;

    virtual void Continue() = 0;
};
