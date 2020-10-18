
#ifndef IMG_READER_IMAGEFILE_READER_H
#define IMG_READER_IMAGEFILE_READER_H

#include "reader.h"

class ImageFileReader : public IReader {
public:
  bool Init(const std::string &cfg) override;
  bool Read(Image &img) override;

private:
  int img_width_, img_height_;
  std::vector<std::string> filelist_;
  int cur_file_idx_;
};

#endif // ifndef IMG_READER_IMAGEFILE_READER_H
