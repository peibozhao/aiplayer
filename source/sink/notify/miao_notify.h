
#pragma once

#include "event_notify.h"
#include "httplib.h"

class MiaoNotify : public IEventNotify {
public:
    MiaoNotify(const std::string &miao);

    bool Init() override;

    bool Notify(const std::string &message) override;

private:
    std::string miao_;

    std::shared_ptr<httplib::Client> client_;
};

