
#include "http_request.h"
#include "common/log.h"
#include "nlohmann/json.hpp"

HttpRequest::HttpRequest(unsigned short port, const std::string &path) {
    port_ = port;
    path_ = path;
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::Init() {
    server_.Post(path_.c_str(), httplib::Server::Handler(std::bind(&HttpRequest::RequestHandler, this, std::placeholders::_1, std::placeholders::_2)));
    return true;
}

void HttpRequest::Start() {
    recv_thread_.reset(new std::thread([this] {
        bool ret = server_.listen("0.0.0.0", port_);
        if (!ret) {
            LOG_ERROR("Http listen failed. %d", port_);
        }
    }));
}

void HttpRequest::Stop() {
}

void HttpRequest::SetCallback(std::function<void (const std::string &)> callback) {
    // std::unique_lock<std::mutex> lock;
    // req_con_.wait(lock, [this] {
    //         return !request_list_.empty();
    //         });
    // std::string ret = request_list_.front();
    // request_list_.pop();
    // return ret;
}

void HttpRequest::RequestHandler(const httplib::Request &req, httplib::Response &res) {
    std::lock_guard<std::mutex> lock(req_mutex_);
    request_list_.push(req.body);
    req_con_.notify_one();
}
