
/*
#include "ocr_http_client.h"
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "spdlog/spdlog.h"
#include "utils/util_functions.h"
#include "utils/util_types.h"
#include "yaml-cpp/yaml.h"

OCRHTTPClient::~OCRHTTPClient() {}

bool OCRHTTPClient::Init(const std::string &cfg) {
    SPDLOG_INFO("OCR config: \n{}", cfg);
    try {
        YAML::Node config = YAML::Load(cfg);
        image_height_ = config["image_height"].as<int>();
        image_width_ = config["image_width"].as<int>();
        ip_port_ = config["host"].as<std::string>();
        api_ = config["path"].as<std::string>();
    } catch (std::exception &e) {
        SPDLOG_ERROR("Catch error: {}", e.what());
        return false;
    }
    return true;
}

bool OCRHTTPClient::SetParam(const std::string &key, const std::string &value) { return true; }

bool OCRHTTPClient::Detect(const std::vector<uint8_t> &data, std::vector<TextBox> &words) {
    TimeLog time_log("OCR Detect");
    words.clear();
    cv::Mat img;
    img.create(cv::Size(image_width_, image_height_), CV_8UC3);
    memcpy(img.data, data.data(), data.size());
    cv::cvtColor(img, img, cv::COLOR_RGB2BGR);

    std::vector<uint8_t> jpg_buffer;
    cv::imencode(".jpg", img, jpg_buffer);
    std::string jpg_base64 = Base64Encode(jpg_buffer);

    std::string request_body;
    request_body.resize(jpg_base64.size());
    for (char jpg_char : jpg_base64) {
        if (jpg_char == '+') {
            request_body.append("%2B");
        } else {
            request_body.push_back(jpg_char);
        }
    }

    httplib::Client cli(ip_port_.c_str());
    // cli.set_read_timeout(10);
    auto resp = cli.Post(api_.c_str(), "img=" + request_body, "application/x-www-form-urlencoded");
    if (!resp || resp->status != 200) {
        SPDLOG_ERROR("Http request error. {}", resp.error());
        return false;
    }
    try {
        nlohmann::json resp_json = nlohmann::json::parse(resp->body);
        auto &data_json = resp_json.at("data").at("raw_out");
        for (auto &ocr_item : data_json) {
            TextBox detect_word;
            std::vector<int> x, y;
            for (int i = 0; i < 4; ++i) {
                x.push_back(ocr_item[0][i][0]);
                y.push_back(ocr_item[0][i][1]);
            }
            detect_word.xmin = *std::min_element(x.begin(), x.end());
            detect_word.xmax = *std::max_element(x.begin(), x.end());
            detect_word.ymin = *std::min_element(y.begin(), y.end());
            detect_word.ymax = *std::max_element(y.begin(), y.end());
            detect_word.text = ocr_item[1].get<std::string>();
            detect_word.text = detect_word.text.substr(detect_word.text.find(' ') + 1);
            detect_word.conf = ocr_item[2].get<float>();
            words.emplace_back(std::move(detect_word));
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Catch exception {}", e.what());
        return false;
    }
    return true;
}
*/
