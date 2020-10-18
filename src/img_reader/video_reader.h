
#ifndef IMG_READER_VIDEO_READER
#define IMG_READER_VIDEO_READER

#include "reader.h"
#include "opencv2/videoio.hpp"

class VideoReader : public IReader {
public:
  bool Init(const std::string &cfg) override;
  bool Read(Image &img) override;

private:
  int video_width_, video_height_;
  std::string video_fn_;
  cv::VideoCapture video_capture_;
};

#endif // ifndef IMG_READER_VIDEO_READER

