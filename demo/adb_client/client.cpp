
#include "glog/logging.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "utils/util_functions.h"
#include <fstream>
#include <iterator>
#include <vector>

int main(int argc, char *argv[]) {
    system("adb shell screencap -p > image.png");
    std::ifstream ifs("image.png", std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(ifs)),
                              std::istreambuf_iterator<char>());

    nlohmann::json request_body;
    request_body["image"]["data"] = Base64Encode(data);

    httplib::Client client("127.0.0.1", 8080);
    client.set_read_timeout(10);
    httplib::Result ret =
        client.Patch("/blhx/players/123", request_body.dump(), "json");
    if (ret.error() != httplib::Error::Success || ret->status != 200) {
        LOG(ERROR) << "Network error. " << int(ret.error());
        return -1;
    } else if (ret->status != 200) {
        LOG(ERROR) << "Request error. " << ret->status;
        return -1;
    }

    if (ret->body.empty()) {
        LOG(ERROR) << ("Response body is empty");
        return -1;
    }
    nlohmann::json response_body = nlohmann::json::parse(ret->body);
    LOG(INFO) << (response_body.dump());
    for (const auto &opt_json : response_body["operations"]) {
        std::string type = opt_json["type"].get<std::string>();
        std::string cmd;
        if (type == "click") {
            cmd = "adb shell input tap " +
                  std::to_string(opt_json["x"].get<int>()) + " " +
                  std::to_string(opt_json["y"].get<int>());
        } else if (type == "swipe") {
            int delta_x = opt_json["delta_x"].get<int>();
            int delta_y = opt_json["delta_y"].get<int>();
            cmd = "adb shell input swipe 1200 600 " +
                  std::to_string(1200 + delta_x) + " " +
                  std::to_string(600 + delta_y);
        }
        if (!cmd.empty()) {
            system(cmd.c_str());
            LOG(INFO) << cmd;
        }
    }
}
