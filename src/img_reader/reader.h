
#ifndef IMG_READER_READER
#define IMG_READER_READER

#include <string>
#include <vector>

#include "opencv2/imgcodecs.hpp"

enum class ImageType {
  RGB_PACKED  // rgbrgbrgb...
};

struct Image {
  ImageType type;
  int height, width;
  std::vector<uint8_t> data;

  Image() {
    type = ImageType::RGB_PACKED;
    height = -1, width = -1;
  }
};

class IReader {
public:
  virtual bool Init(const std::string &cfg) = 0;
  virtual bool Read(Image &img) = 0;
};

Image CvImageToImage(const cv::Mat &cv_img);
cv::Mat ImageToCvImage(const Image &img);

#endif // ifndef IMG_READER_READER

