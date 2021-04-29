
#include "httplib.h"
#include <fstream>
#include <vector>
#include <iterator>
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include "fmt/format.h"
#include "utils/util_functions.h"

int main(int argc, char *argv[]) {
    system("adb shell screencap -p > image.png");
    std::ifstream ifs("image.png", std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    nlohmann::json request_body;
    request_body["image"]["data"] = Base64Encode(data);

    httplib::Client client("127.0.0.1", 8080);
    client.set_read_timeout(10);
    httplib::Result ret = client.Patch("/blhx/players/123", request_body.dump(), "json");
    if (ret.error() != httplib::Success || ret->status != 200) {
        SPDLOG_ERROR("Network error. {}", ret.error());
        return -1;
    } else if (ret->status != 200) {
        SPDLOG_ERROR("Request error. {}", ret->status);
        return -1;
    }

    if (ret->body.empty()) {
        SPDLOG_ERROR("Response body is empty");
        return -1;
    }
    nlohmann::json response_body = nlohmann::json::parse(ret->body);
    SPDLOG_INFO(response_body.dump());
    for (const auto &opt_json : response_body["operations"]) {
        std::string type = opt_json["type"].get<std::string>();
        std::string cmd;
        if (type == "click") {
            cmd = fmt::format("adb shell input tap {} {}", opt_json["x"].get<int>(), opt_json["y"].get<int>());
        } else if (type == "swipe") {
            int delta_x = opt_json["delta_x"].get<int>();
            int delta_y = opt_json["delta_y"].get<int>();
            cmd = fmt::format("adb shell input swipe {} {} {} {}", 1200, 600, 1200 + delta_x, 600 + delta_y);
        }
        if (!cmd.empty()) {
            system(cmd.c_str());
            SPDLOG_INFO(cmd);
        }
    }
}

