
#pragma once

#include "request.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include "httplib.h"

// Return json format
class HttpRequest : public IRequest {
public:
    HttpRequest(unsigned short port, const std::string &path);

    ~HttpRequest() override;

    bool Init() override;

    void Start() override;

    void Stop() override;

    void SetCallback(std::function<void(const std::string &json_str)> callback) override;

private:
    void RequestHandler(const httplib::Request &req, httplib::Response &res);

private:
    unsigned short port_;
    std::string path_;
    httplib::Server server_;

    std::mutex req_mutex_;
    std::condition_variable req_con_;
    std::queue<std::string> request_list_;

    std::shared_ptr<std::thread> recv_thread_;
};
