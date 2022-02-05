
#include "miao_notify.h"
#include "utils/log.h"
#include "nlohmann/json.hpp"

MiaoNotify::MiaoNotify(const std::string &miao) { miao_ = miao; }

bool MiaoNotify::Init() {
    client_.reset(new httplib::Client("http://miaotixing.com"));
    return true;
}

bool MiaoNotify::Notify(const std::string &message) {
    std::string request_body;
    request_body += "id=" + miao_;
    request_body += "&text=" + message;
    request_body += "&type=json";

    int retry_count = 2;
    while (retry_count--) {
        httplib::Result http_ret =
            client_->Post("/trigger", request_body.c_str(),
                          "application/x-www-form-urlencoded");
        if (http_ret->status != 200) {
            LOG_ERROR("Notify http failed %d %s", http_ret->status,
                      http_ret->reason.c_str());
            continue;
        }
        std::string res_body = http_ret->body;
        nlohmann::json res_json = nlohmann::json::parse(res_body);
        int miao_code = res_json["code"].get<int>();
        if (miao_code != 0) {
            LOG_ERROR("Notify failed %d %s", miao_code,
                      res_json["msg"].get<std::string>().c_str());

            if (miao_code == 102) {
                std::this_thread::sleep_for(std::chrono::seconds(
                    res_json["data"]["remaining"].get<int>() + 1));
            }
            continue;
        }
        return true;
    }
    return false;
}
