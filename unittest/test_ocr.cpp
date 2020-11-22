
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <thread>
#include "catch2/catch.hpp"
#include "spdlog/spdlog.h"
#include "opencv2/opencv.hpp"
#include "ocr/ocr_http_client.h"
#include "img_reader/reader.h"

void StartOCRServer() {
  SPDLOG_INFO("OCR Server start ... ");
  int fork_ret = fork();
  if (fork_ret < 0) {
    SPDLOG_ERROR("fork failed");
    exit(1);
  } else if (fork_ret == 0) {
    int outfd = open("ocr.log", O_CREAT|O_WRONLY|O_TRUNC, 0644);
    dup2(outfd, STDOUT_FILENO);
    dup2(outfd, STDERR_FILENO);
    chdir("/home/peibozhao/Code/chineseocr_lite");
    execlp("python", "python", "/home/peibozhao/Code/chineseocr_lite/backend/main.py", nullptr);
  } else {
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
}

void StopOCRServer() {
  system(R"(
      OCR_PID=`ps aux | grep backend | grep -v grep | awk '{print $2}'`;
      if [ -n "${OCR_PID}" ]; then
        kill -9 ${OCR_PID};
      fi
      )");
  std::this_thread::sleep_for(std::chrono::seconds(3));
}

TEST_CASE("OCR") {
  SECTION("HTTPClient") {
    StartOCRServer();
    OCRHTTPClient *ocr = new OCRHTTPClient();
    ocr->Init(R"(
    image_width: 2340
    image_height: 1080
    host: 127.0.0.1:8089
    path: /api/tr-run/
    )");
    cv::Mat cv_img = cv::imread("img.png");
    REQUIRE(!cv_img.empty());
    Image img = CvImageToImage(cv_img);
    std::vector<DetectWord> words;
    ocr->Detect(img.data, words);
    for (auto detect_word : words) {
      SPDLOG_INFO("{}: {},{},{},{} {}", detect_word.word, detect_word.xmin,
                   detect_word.ymin, detect_word.xmax, detect_word.ymax,
                   detect_word.conf);
    }
    StopOCRServer();
  }
}
