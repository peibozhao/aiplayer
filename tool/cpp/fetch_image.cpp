
#include "source/minicap_source.cpp"
#include "common/log.h"
#include <fstream>

int main() {
    MinicapSource minicap("127.0.0.1", 1313);
    if (!minicap.Init()) {
        LOG_ERROR("Minicap init failed");
        return -1;
    }

    std::vector<char> image_buffer = minicap.GetImageBuffer();
    std::ofstream ofs("test.jpg", std::ios::binary);
    ofs.write((char *)image_buffer.data(), image_buffer.size());

    return 0;
}
