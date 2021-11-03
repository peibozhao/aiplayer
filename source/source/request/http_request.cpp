
#include "http_request.h"
#include "common/log.h"
#include "nlohmann/json.hpp"

HttpRequest::HttpRequest(const std::string &ip, unsigned short port) {
    ip_ = ip;
    port_ = port;
}

HttpRequest::~HttpRequest() { Stop(); }

bool HttpRequest::Init() {
    LOG_INFO("Http server listen %s:%d%s", ip_.c_str(), port_, "/config");
    server_.Get("/config",
                httplib::Server::Handler(std::bind(&HttpRequest::RequestHandler, this,
                                                   std::placeholders::_1, std::placeholders::_2)));
    return true;
}

void HttpRequest::Start() {
    LOG_INFO("Http server start");
    recv_thread_.reset(new std::thread([this] {
        bool ret = server_.listen(ip_.c_str(), port_);
        if (!ret) { LOG_ERROR("Http listen failed. %d", port_); }
    }));
}

void HttpRequest::Stop() {
    LOG_INFO("Http server stop");
    server_.stop();
}

void HttpRequest::SetCallback(const std::string &path, RequestCallback callback) {
    LOG_INFO("Register http callback %s", path.c_str());
    callbacks_[path] = callback;
}

void HttpRequest::RequestHandler(const httplib::Request &req, httplib::Response &res) {
    std::string path = req.path;
    LOG_INFO("Trigger callback %s", path.c_str());
    auto iter = callbacks_.find(path);
    if (iter != callbacks_.end()) {
        RequestCallback callback = iter->second;
        nlohmann::json req_root;
        for (auto param : req.params) {
            req_root[param.first] = param.second;
        }
        if (!callback(req_root.dump())) {
            res.reason = "Callback failed";
            res.status = 400;
            LOG_INFO("%s", res.reason.c_str());
        } else {
            res.status = 200;
        }
    } else {
        res.status = 400;
        res.reason = "No implementation";
        LOG_INFO("%s", res.reason.c_str());
    }
}
