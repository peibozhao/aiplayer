
#pragma once

#include "request.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include "httplib.h"

// Return json format
class HttpRequest : public IRequest {
public:
    typedef std::function<bool(const std::string &)> RequestCallback;

public:
    HttpRequest(const std::string &ip, unsigned short port);

    ~HttpRequest() override;

    bool Init() override;

    void Start() override;

    void Stop() override;

    void SetCallback(const std::string &path, RequestCallback callback) override;

private:
    void RequestHandler(const httplib::Request &req, httplib::Response &res);

private:
    std::string ip_;
    unsigned short port_;
    httplib::Server server_;

    std::shared_ptr<std::thread> recv_thread_;
    std::map<std::string, RequestCallback> callbacks_;
};
