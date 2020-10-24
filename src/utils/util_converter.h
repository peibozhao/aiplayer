
#ifndef UTILS_UTIL_CONVERTER_H
#define UTILS_UTIL_CONVERTER_H

#include "object_detect/detect.h"
#include "ocr/ocr.h"
#include "player/player.h"

DetectObject Converter(const DetectBox &box);
std::vector<DetectObject> Converter(const std::vector<DetectBox> &boxs);

DetectObject Converter(const DetectWord &word);
std::vector<DetectObject> Converter(const std::vector<DetectWord> &words);

#endif // ifndef UTILS_UTIL_CONVERTER_H
