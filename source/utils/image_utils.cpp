
#include "image_utils.h"
#include <cassert>
#include <netinet/in.h>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"

FrameSize ImageSize(const Image &image) {
    assert(image.format == ImageFormat::YUV420);
    // uint16_t width = ntohs(*(uint16_t *)&image.buffer[0]);
    // uint16_t height = ntohs(*(uint16_t *)&image.buffer[2]);
    uint16_t width = *(uint16_t *)&image.buffer[0];
    uint16_t height = *(uint16_t *)&image.buffer[2];
    return std::make_pair(width, height);
}

Image Convert(const Image &image, ImageFormat dst_format) {
    if (image.format == dst_format) {
        return image;
    } else if (image.format == ImageFormat::YUV420) {
        cv::Mat yuv_mat, bgr_mat;
        FrameSize frame_size = ImageSize(image);
        yuv_mat.create(frame_size.second * 3 / 2, frame_size.first, CV_8UC1);
        memcpy(yuv_mat.data, image.buffer.data(), image.buffer.size());
        cv::cvtColor(yuv_mat, bgr_mat, cv::COLOR_YUV420p2RGB);

        Image ret;
        ret.format = ImageFormat::JPEG;
        cv::imencode(".jpg", bgr_mat, ret.buffer);
        return ret;
    } else {
        assert(false && "Unsupport image convert");
        return Image();
    }
}
