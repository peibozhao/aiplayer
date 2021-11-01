
#include "paddle_ocr.h"
#include "common/log.h"
#include "common/util_functions.h"
#include "nlohmann/json.hpp"
#include <cassert>

PaddleOcr::PaddleOcr(const std::string &host, unsigned short port) {
    host_ = host;
    port_ = port;
}

PaddleOcr::PaddleOcr(const std::string &host, unsigned short port, int timeout)
    : PaddleOcr(host, port) {
    recv_timeout_ = timeout;
}

PaddleOcr::~PaddleOcr() {}

bool PaddleOcr::Init() {
    client_.reset(new httplib::Client(host_, port_));
    if (recv_timeout_) {
        client_->set_read_timeout(recv_timeout_.value());
    }
    return true;
}

std::vector<TextBox> PaddleOcr::Detect(const ImageInfo &image_info,
                                       const std::vector<char> &buffer) {
    assert(image_info.format == ImageFormat::JPEG);

    std::vector<uint8_t> buffer_uint8(buffer.size());
    std::transform(buffer.begin(), buffer.end(), buffer_uint8.begin(),
                   [](char c) { return static_cast<uint8_t>(c); });
    std::string image_base64 = Base64Encode(buffer_uint8);

    nlohmann::json request_json;
    request_json["images"].push_back(image_base64);

    httplib::Result http_ret =
        client_->Post("/predict/ocr_system", request_json.dump(), "application/json");

    if (http_ret.error() != httplib::Error::Success) {
        LOG_ERROR("Http return error %d", static_cast<int>(http_ret.error()));
        return {};
    }

    nlohmann::json http_body = nlohmann::json::parse(http_ret->body);

    if (http_body["results"].empty()) { return {}; }
    nlohmann::json ocr_texts_json = http_body["results"].front();
    std::vector<TextBox> ret;
    for (const auto &ocr_text_json : ocr_texts_json) {
        TextBox text_box;
        text_box.text = ocr_text_json["text"].get<std::string>();
        int left = 0, right = 0, bottom = 0, top = 0;
        int point_count = ocr_text_json["text_region"].size();
        for (int point_idx = 0; point_idx < point_count; ++point_idx) {
            int x = ocr_text_json["text_region"][point_idx][0].get<int>();
            int y = ocr_text_json["text_region"][point_idx][1].get<int>();
            right = x > right ? x : right;
            bottom = y > bottom ? y : bottom;
            left = x < left ? x : left;
            top = y < top ? y : top;
            text_box.x += x;
            text_box.y += y;
        }
        text_box.width = right - left;
        text_box.height = bottom - top;
        text_box.x /= point_count;
        text_box.y /= point_count;
        ret.push_back(text_box);
        LOG_DEBUG("%s %d %d", text_box.text.c_str(), text_box.x, text_box.y);
    }
    return ret;
}
