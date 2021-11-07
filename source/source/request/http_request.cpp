
#include "http_request.h"
#include "common/log.h"
#include "common/util_functions.h"
#include "nlohmann/json.hpp"

std::map<std::string, RequestOperation> HttpRequest::httpmethod_to_ops_ = {
    {"GET", RequestOperation::Query},
    {"POST", RequestOperation::Add},
    {"DELETE", RequestOperation::Delete},
    {"PATCH", RequestOperation::Modify},
    {"PUT", RequestOperation::Replace}
};

HttpRequest::HttpRequest(const std::string &ip, unsigned short port) {
    ip_ = ip;
    port_ = port;
}

HttpRequest::~HttpRequest() { Stop(); }

bool HttpRequest::Init() { return true; }

void HttpRequest::Start() {
    LOG_INFO("Http server start");
    recv_thread_.reset(new std::thread([this] {
        LOG_INFO("Http server listen %s:%d", ip_.c_str(), port_);
        bool ret = server_.listen(ip_.c_str(), port_);
        if (!ret) { LOG_ERROR("Http listen failed. %d", port_); }
    }));
}

void HttpRequest::Stop() {
    LOG_INFO("Http server stop");
    server_.stop();
    recv_thread_->join();
}

void HttpRequest::SetCallback(const std::string &path, RequestOperation op,
                              RequestCallback callback) {
    auto op_to_methods = ReverseMap(httpmethod_to_ops_);
    LOG_INFO("Register http callback %s %s", op_to_methods[op].c_str(), path.c_str());
    auto request_handler = httplib::Server::Handler(std::bind(
        &HttpRequest::RequestHandler, this, std::placeholders::_1, std::placeholders::_2));
    switch (op) {
    case RequestOperation::Add:
        server_.Post(path.c_str(), request_handler);
        break;
    case RequestOperation::Delete:
        server_.Delete(path.c_str(), request_handler);
        break;
    case RequestOperation::Modify:
        server_.Patch(path.c_str(), request_handler);
        break;
    case RequestOperation::Query:
        server_.Get(path.c_str(), request_handler);
        break;
    case RequestOperation::Replace:
        server_.Put(path.c_str(), request_handler);
        break;
    default:
        LOG_ERROR("Unkown request opration type");
        return;
    }
    callbacks_[std::make_pair(path, op)] = callback;
}

void HttpRequest::RequestHandler(const httplib::Request &req, httplib::Response &res) {
    RequestOperation op = httpmethod_to_ops_[req.method];
    std::string path = req.path;
    LOG_INFO("Trigger callback %s %s", req.method.c_str(), path.c_str());
    auto iter = callbacks_.find(std::make_pair(path, op));
    if (iter != callbacks_.end()) {
        RequestCallback callback = iter->second;
        if (!callback(req.body, res.body)) {
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
