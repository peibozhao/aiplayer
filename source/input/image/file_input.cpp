
#include "file_input.h"
#include "utils/log.h"
#include <fstream>

FileImageInput::FileImageInput(const std::string &fname,
                         const ImageFormat &image_format) {
    fname_ = fname;
    format_ = image_format;
}

bool FileImageInput::Init() {
    std::ifstream ifs(fname_, std::ios::binary);
    if (!ifs.is_open()) {
        LOG_ERROR("File open failed: %s", fname_.c_str());
        return false;
    }
    ifs.seekg(0, std::ios::end);
    int file_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    buffer_.resize(file_size);
    ifs.read((char *)buffer_.data(), buffer_.size());
    return true;
}

Image FileImageInput::GetOneFrame() {
    Image ret;
    ret.buffer = buffer_;
    ret.format = format_;
    return ret;
}
