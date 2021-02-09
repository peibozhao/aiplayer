
#include "util_functions.h"
#include <list>
#include <math.h>

float Sigmoid(float x) { return 1 / (1 + std::exp(-x)); }

std::vector<std::vector<float>> NMS(const std::vector<std::vector<float>> &boxes, float thresh) {
    std::vector<std::vector<float>> ret;
    std::list<std::vector<float>> boxes_list(boxes.begin(), boxes.end());
    while (!boxes_list.empty()) {
        auto max_iter = boxes_list.begin();
        for (auto iter = boxes_list.begin(); iter != boxes_list.end(); ++iter) {
            if ((*iter)[4] > (*max_iter)[4]) {
                max_iter = iter;
            }
        }
        ret.push_back(*max_iter);
        boxes_list.erase(max_iter);
        for (auto iter = boxes_list.begin(); iter != boxes_list.end();) {
            float iou = CalcIoU(ret[ret.size() - 1], *iter);
            if (iou > thresh) {
                iter = boxes_list.erase(iter);
            } else {
                ++iter;
            }
        }
    }
    return ret;
}

float CalcIoU(const std::vector<float> &lhs, const std::vector<float> &rhs) {
    float lhs_area = RectArea(lhs);
    float rhs_area = RectArea(rhs);
    float temp_area1 = RectArea({lhs[0], lhs[1], rhs[2], rhs[3]});
    float temp_area2 = RectArea({rhs[0], rhs[1], lhs[2], lhs[3]});
    float intersection_area = std::min(temp_area1, temp_area2);
    if (intersection_area < 0) {
        return 0;
    }
    float union_area = lhs_area + rhs_area - intersection_area;
    return intersection_area / union_area;
}

float RectArea(const std::vector<float> &rect) { return (rect[2] - rect[0]) * (rect[3] - rect[1]); }

bool HandleSystemResult(int sys_ret) { return WIFEXITED(sys_ret) && (WEXITSTATUS(sys_ret) == 0); }

const static std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";

std::string Base64Encode(const std::vector<uint8_t> &data) {
    std::string ret;
    std::vector<uint8_t> input_chars_3;
    std::array<uint8_t, 4> output_chars_4;

    for (uint8_t cur_char : data) {
        input_chars_3.push_back(cur_char);
        if (input_chars_3.size() == 3) {
            output_chars_4[0] = (input_chars_3[0] & 0xfc) >> 2;
            output_chars_4[1] = ((input_chars_3[0] & 0x03) << 4) + ((input_chars_3[1] & 0xf0) >> 4);
            output_chars_4[2] = ((input_chars_3[1] & 0x0f) << 2) + ((input_chars_3[2] & 0xc0) >> 6);
            output_chars_4[3] = input_chars_3[2] & 0x3f;
            for (uint8_t output_char : output_chars_4) {
                ret += base64_chars[output_char];
            }
            input_chars_3.clear();
        }
    }

    if (!input_chars_3.empty()) {
        int input_size = input_chars_3.size();
        input_chars_3.resize(3, 0);
        output_chars_4[0] = (input_chars_3[0] & 0xfc) >> 2;
        output_chars_4[1] = ((input_chars_3[0] & 0x03) << 4) + ((input_chars_3[1] & 0xf0) >> 4);
        output_chars_4[2] = ((input_chars_3[1] & 0x0f) << 2) + ((input_chars_3[2] & 0xc0) >> 6);
        output_chars_4[3] = input_chars_3[2] & 0x3f;

        for (int i = 0; i < input_size + 1; ++i) {
            ret += base64_chars[output_chars_4[i]];
        }
        while (input_size++ < 3) {
            ret += '=';
        }
    }
    return ret;
}

std::string Base64Decode(const std::string &data) {
    size_t in_len = base64_chars.size();
    int i = 0, j = 0, in = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (base64_chars[in] != '=')) {
        char_array_4[i++] = base64_chars[in];
        in++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_chars.find(char_array_4[i]) & 0xff;
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++) {
                ret += char_array_3[i];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++) {
            char_array_4[j] = base64_chars.find(char_array_4[j]) & 0xff;
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; (j < i - 1); j++) {
            ret += char_array_3[j];
        }
    }
    return ret;
}

float CalcRadian(const std::vector<float> &p1, const std::vector<float> &o,
                 const std::vector<float> &p2) {
    float l1[2] = {p2[0] - o[0], p2[1] - o[1]};
    float l2[2] = {p2[0] - o[0], p2[1] - o[1]};
    float dot_ret = l1[0] * l2[0] + l1[1] * l2[1];
    float l1_len = std::sqrt(std::pow(l1[0], 2) + std::pow(l1[1], 2));
    float l2_len = std::sqrt(std::pow(l2[0], 2) + std::pow(l2[1], 2));
    float cos_ret = dot_ret / (l1_len * l2_len);
    return std::acos(cos_ret);
}
