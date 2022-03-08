
#pragma once

#include <functional>
#include <string>

enum RequestOperation { Add, Delete, Modify, Query, Replace };

class IRequest {
public:
  typedef std::function<bool(const std::string &, std::string &)>
      RequestCallback;

public:
  virtual ~IRequest() {}

  virtual bool Init() = 0;

  virtual void SetCallback(const std::string &key, RequestOperation op,
                           RequestCallback callback) = 0;
};
