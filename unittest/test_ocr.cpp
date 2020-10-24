
#include <fstream>
#include "catch2/catch.hpp"
#include "spdlog/spdlog.h"
#include "ocr/ocr_http_client.h"

TEST_CASE("OCR") {
  SECTION("HTTPClient") {
    OCRHTTPClient *ocr = new OCRHTTPClient();
    ocr->Init("");
    std::ifstream ifs("./828706969.rgb", std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    spdlog::info("{}", data.size());
    auto detect_words = ocr->Detect(data);
    for (auto detect_word : detect_words) {
      spdlog::info("{}: {},{},{},{} {}", detect_word.word, detect_word.xmin,
                   detect_word.ymin, detect_word.xmax, detect_word.ymax,
                   detect_word.conf);
    }
  }
}
