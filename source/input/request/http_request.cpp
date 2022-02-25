
#include "http_request.h"
#include "common/log.h"
#include "nlohmann/json.hpp"
#include "utils/util_functions.h"

std::map<std::string, RequestOperation> HttpRequest::httpmethod_to_ops_ = {
    {"GET", RequestOperation::Query},
    {"POST", RequestOperation::Add},
    {"DELETE", RequestOperation::Delete},
    {"PATCH", RequestOperation::Modify},
    {"PUT", RequestOperation::Replace}};

HttpRequest::HttpRequest(const std::string &ip, unsigned short port) {
    ip_ = ip;
    port_ = port;
}

HttpRequest::~HttpRequest() {
    LOG(INFO) << "Http server stop";
    server_.stop();
    recv_thread_->join();
}

bool HttpRequest::Init() {
    LOG(INFO) << "Http server start";
    recv_thread_.reset(new std::thread([this] {
        LOG(INFO) << "Http server listen " << ip_ << ":" << port_;
        bool ret = server_.listen(ip_.c_str(), port_);
        if (!ret) {
            LOG(ERROR) << "Http listen failed. " << port_;
        }
    }));
    return true;
}

void HttpRequest::SetCallback(const std::string &path, RequestOperation op,
                              RequestCallback callback) {
    auto op_to_methods = ReverseMap(httpmethod_to_ops_);
    LOG(INFO) << "Register http callback " << op_to_methods[op] << " " << path;
    auto request_handler = httplib::Server::Handler(
        std::bind(&HttpRequest::RequestHandler, this, std::placeholders::_1,
                  std::placeholders::_2));
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
        LOG(ERROR) << "Unkown request opration type";
        return;
    }
    callbacks_[std::make_pair(path, op)] = callback;
}

void HttpRequest::RequestHandler(const httplib::Request &req,
                                 httplib::Response &res) {
    RequestOperation op = httpmethod_to_ops_[req.method];
    std::string path = req.path;
    LOG(INFO) << "Trigger callback " << req.method << " " << path;
    auto iter = callbacks_.find(std::make_pair(path, op));
    if (iter != callbacks_.end()) {
        RequestCallback callback = iter->second;
        if (!callback(req.body, res.body)) {
            res.reason = "Callback failed";
            res.status = 400;
            LOG(ERROR) << res.reason;
        } else {
            res.status = 200;
        }
    } else {
        res.status = 400;
        res.reason = "No implementation";
        LOG(ERROR) << res.reason;
    }
}
