
#include "image_source.h"
#include <fstream>
#include "common/log.h"

ImageSource::ImageSource(const std::string &fname, const ImageInfo &image_info) {
    fname_ = fname;
    image_info_ = image_info;
}

bool ImageSource::Init() {
    std::ifstream ifs(fname_, std::ios::binary);
    if (!ifs.is_open()) {
        LOG_ERROR("File open failed: %s", fname_.c_str());
        return false;
    }
    ifs.seekg(0, std::ios::end);
    int file_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    image_buffer_.resize(file_size);
    ifs.read(image_buffer_.data(), image_buffer_.size());
    return true;
}

ImageInfo ImageSource::GetImageInfo() {
    return image_info_;
}

std::vector<char> ImageSource::GetImageBuffer() {
    return image_buffer_;
}
