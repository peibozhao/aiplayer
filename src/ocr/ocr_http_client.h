
#ifndef OCR_OCR_HTTP_CLIENT_H
#define OCR_OCR_HTTP_CLIENT_H

#include "ocr.h"

class OCRHTTPClient : public IOCR {
public:
  ~OCRHTTPClient() override;

  bool Init(const std::string &cfg) override;

  bool SetParam(const std::string &key, const std::string &value) override;

  std::vector<DetectWord> Detect(const std::vector<uint8_t> &data) override;

private:
  int img_width_, img_height_;
  std::string ip_port_;
  std::string api_;
};


#endif // ifndef OCR_OCR_HTTP_CLIENT_H
