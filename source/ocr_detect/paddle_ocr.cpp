
#include "paddle_ocr.h"
#include <cassert>
#include "nlohmann/json.hpp"
#include "httplib.h"
#include "common/util_functions.h"

PaddleOcr::PaddleOcr(const std::string &ip, unsigned short port) {
}

PaddleOcr::~PaddleOcr() {
}

std::vector<TextBox> PaddleOcr::Detect(ImageFormat format, const std::vector<char> &buffer) {
    assert(format == ImageFormat::JPEG);

    std::string image_base64 = Base64Encode(std::transform(buffer));

    nlohmann::json request_json;
    request_json["images"].push_back(
}
