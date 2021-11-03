
#pragma once

#include <vector>
#include <array>
#include <map>

float Sigmoid(float x);

std::vector<std::vector<float>> NMS(const std::vector<std::vector<float>> &boxes, float thresh);

float CalcIoU(const std::vector<float> &lhs, const std::vector<float> &rhs);

float RectArea(const std::vector<float> &rect);

bool HandleSystemResult(int sys_ret);

std::string Base64Encode(const std::vector<uint8_t> &data);

std::vector<uint8_t> Base64Decode(const std::string &data);

float CalcRadian(const std::vector<float> &p1, const std::vector<float> &o, const std::vector<float> &p2);

template <typename T1, typename T2>
std::map<T2, T1> ReverseMap(const std::map<T1, T2> &m) {
    std::map<T2, T1> ret;
    for (auto ele : m) {
        ret[ele.second] = ele.first;
    }
    return ret;
}
