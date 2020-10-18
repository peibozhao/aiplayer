
#ifndef IMG_READER_ANDROID_REMOTE_SCREENSHOT_READER_H
#define IMG_READER_ANDROID_REMOTE_SCREENSHOT_READER_H

#include "reader.h"

class AndroidRemoteScreenshotReader : public IReader {
public:
  bool Init(const std::string &cfg) override;
  bool Read(Image &img) override;

private:
  std::string remote_storage_fn_;
  std::string local_storage_fn_;
  std::string screenshot_cmd_;
  std::string pull_img_cmd_;
};

#endif // ifndef IMG_READER_ANDROID_SCREENSHOT_READER_H
