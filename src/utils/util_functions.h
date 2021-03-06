
#ifndef UTILS_UTILS_FUNCTIONS
#define UTILS_UTILS_FUNCTIONS

#include <vector>
#include <array>

float Sigmoid(float x);

std::vector<std::vector<float>> NMS(const std::vector<std::vector<float>> &boxes, float thresh);

float CalcIoU(const std::vector<float> &lhs, const std::vector<float> &rhs);

float RectArea(const std::vector<float> &rect);

bool HandleSystemResult(int sys_ret);

std::string Base64Encode(const std::vector<uint8_t> &data);

float CalcRadian(const std::vector<float> &p1, const std::vector<float> &o, const std::vector<float> &p2);

#endif // ifndef UTILS_UTILS_FUNCTIONS
