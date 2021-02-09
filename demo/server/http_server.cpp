
#include "http_server.h"
#include <linux/uuid.h>

void HttpServer::Init(const std::string &config_str) {
    ip_ = "0.0.0.0";
    port_ = 8080;
}

void HttpServer::Start() {
    server_.listen(ip_.c_str(), port_);
}

void HttpServer::Stop() {}

void HttpServer::InitHttplibServer() {
    server_.Post("/players",
               [this](const httplib::Request &request, httplib::Response &response) {
               });
}

