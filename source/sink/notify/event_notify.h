
#pragma once

#include <string>

class IEventNotify {
public:
    virtual bool Init() { return true; }

    virtual bool Notify(const std::string &message) = 0;
};

