
#pragma once

#include <string>

class IApplication {
public:
    virtual bool Init() { return true; }

    virtual void Run() = 0;

    virtual void Pause() = 0;

    virtual void Continue() = 0;

    virtual void Stop() = 0;

    virtual bool SetParam(const std::string &key, const std::string &value) { return false; }

    virtual std::string GetParam(const std::string &key) { return ""; }
};
