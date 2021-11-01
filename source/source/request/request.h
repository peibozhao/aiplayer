
#pragma once

#include <string>
#include <functional>

class IRequest {
public:
    virtual ~IRequest() {}

    virtual bool Init() { return true; }

    virtual void Start() {}

    virtual void Stop() {}

    virtual void SetCallback(std::function<void(const std::string &json_str)> callback) = 0;
};
