
#include "miao_notify.h"
#include "common/log.h"
#include "nlohmann/json.hpp"

MiaoNotify::MiaoNotify(const std::string &miao) { miao_ = miao; }

bool MiaoNotify::Init() {
  client_.reset(new httplib::Client("http://miaotixing.com"));
  return true;
}

bool MiaoNotify::Notify(const std::string &message) {
  DLOG(INFO) << "Notify: " << message;
  std::string request_body;
  request_body += "id=" + miao_;
  request_body += "&text=" + message;
  request_body += "&type=json";

  httplib::Result http_ret = client_->Post("/trigger", request_body.c_str(),
                                           "application/x-www-form-urlencoded");
  if (!http_ret) {
    LOG(ERROR) << "Notify http failed. " << static_cast<int>(http_ret.error());
    return false;
  }
  if (http_ret->status != 200) {
    LOG(ERROR) << "Notify http failed. " << http_ret->status << " "
               << http_ret->reason;
    return false;
  }
  std::string res_body = http_ret->body;
  nlohmann::json res_json = nlohmann::json::parse(res_body);
  int miao_code = res_json["code"].get<int>();
  if (miao_code != 0) {
    LOG(ERROR) << "Notify failed " << miao_code << " "
               << res_json["msg"].get<std::string>();

    if (miao_code == 102) {
      // Retry
      int remain_seconds = res_json["data"]["remaining"].get<int>();
      LOG(INFO) << "Sleep " << remain_seconds << " to retry";
      std::this_thread::sleep_for(std::chrono::seconds(remain_seconds + 1));
      http_ret = client_->Post("/trigger", request_body.c_str(),
                               "application/x-www-form-urlencoded");
    }
  }
  return true;
}
