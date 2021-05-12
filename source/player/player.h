
#pragma once

#include "object_detect/object_detect.h"
#include "ocr_detect/ocr_detect.h"
#include <vector>

enum class PlayOperationType {
    NONE,         // 没有操作
    SLEEP,        // 等待
    SCREEN_CLICK, // 触屏点击操作
    SCREEN_SWIPE, // 触屏滑动操作
    LIMITS,       // 特殊符号. 到达限制
};

struct SleepOperation {
    int time; // ms
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

    PlayOperation(PlayOperationType type = PlayOperationType::NONE) {
        this->type = type;
    }
};

class IPlayer {
public:
    virtual ~IPlayer() {}

    virtual bool Init(const std::string &config_str) { return true; };

    virtual std::vector<PlayOperation>
    Play(const std::vector<ObjectBox> &objects,
         const std::vector<TextBox> &texts) = 0;

    virtual bool GetLimit() { return false; }
};

inline PlayOperation Click(const std::vector<ObjectBox> &objects,
                           const std::string &name) {
    PlayOperation opt;
    opt.type = PlayOperationType::NONE;
    for (const ObjectBox &object : objects) {
        if (object.name == name) {
            opt.type = PlayOperationType::SCREEN_CLICK;
            opt.click.x = object.x + object.width / 2;
            opt.click.y = object.y + object.height / 2;
            break;
        }
    }
    return opt;
}

inline PlayOperation Click(const std::vector<TextBox> &texts,
                           const std::string &name) {
    PlayOperation opt;
    opt.type = PlayOperationType::NONE;
    for (const TextBox &text : texts) {
        if (text.text == name) {
            opt.type = PlayOperationType::SCREEN_CLICK;
            opt.click.x = text.x + text.width / 2;
            opt.click.y = text.y + text.height / 2;
            break;
        }
    }
    return opt;
}
