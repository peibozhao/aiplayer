
#pragma once

#include "httplib.h"
#include "request.h"
#include <condition_variable>
#include <mutex>
#include <queue>

// Return json format
class HttpRequest : public IRequest {
public:
  HttpRequest(const std::string &ip, unsigned short port);

  ~HttpRequest() override;

  bool Init() override;

  void SetCallback(const std::string &path, RequestOperation op,
                   RequestCallback callback) override;

private:
  void RequestHandler(const httplib::Request &req, httplib::Response &res);

private:
  static std::map<std::string, RequestOperation> httpmethod_to_ops_;

  std::string ip_;
  unsigned short port_;
  httplib::Server server_;

  std::shared_ptr<std::thread> recv_thread_;
  std::map<std::pair<std::string, RequestOperation>, RequestCallback>
      callbacks_;
};
