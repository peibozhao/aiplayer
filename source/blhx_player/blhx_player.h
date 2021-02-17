/**
 * @file player.h
 * @brief
 * @author peibozhao
 * @version 1.0.0
 * @date 2020-10-13
 */

#pragma once

#include <string>
#include <vector>
#include "object_detect/object_detect.h"
#include "ocr_detect/ocr_detect.h"
#include "utils/util_defines.h"

enum class PlayOperationType {
    NONE,         // 没有操作
    SLEEP,        // 等待
    SCREEN_CLICK, // 触屏点击操作
    SCREEN_SWIPE, // 触屏滑动操作
    LIMITS,       // 特殊符号. 到达限制
};

struct SleepOperation {
    int time;  // ms
    SleepOperation() : time(0) {}
};

struct ClickOperation {
    int x, y;
    ClickOperation() : x(-1), y(-1) {}
};

struct SwipeOperation {
    int delta_x, delta_y;
    SwipeOperation() : delta_x(-1), delta_y(-1) {}
};

struct PlayOperation {
    PlayOperationType type;
    union {
        SleepOperation sleep;
        ClickOperation click;
        SwipeOperation swipe;
    };

    PlayOperation(PlayOperationType type = PlayOperationType::NONE) { this->type = type; }
};

class IBLHXPlayer {
public:
    virtual ~IBLHXPlayer() {}

    InitialBaseDefine

    virtual std::vector<PlayOperation> Play(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) = 0;

    virtual bool GetLimit() { return false; }
};

