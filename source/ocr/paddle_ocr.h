
#pragma once

#include "httplib.h"
#include "ocr.h"

class PaddleOcr : public IOcrDetect {
public:
  PaddleOcr(const std::string &host, unsigned short port);

  PaddleOcr(const std::string &host, unsigned short port, int timeout);

  ~PaddleOcr() override;

  bool Init() override;

  std::vector<TextBox> Detect(const cv::Mat &image) override;

private:
  std::string host_;
  unsigned short port_;
  std::shared_ptr<httplib::Client> client_;
  std::optional<int> recv_timeout_;
};
