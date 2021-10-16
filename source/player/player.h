
#pragma once

#include "object_detect/object_detect.h"
#include "ocr_detect/ocr_detect.h"
#include <vector>

enum class PlayOperationType {
    SCREEN_CLICK = 1, // 触屏点击操作
    SLEEP,     // 等待
    LIMITS = 1000, // 特殊符号. 到达限制
};

struct ClickOperation {
    int x, y;
    ClickOperation(int xi = 0, int yi = 0) : x(xi), y(yi) {}
};

// struct SwipeOperation {
//     int delta_x, delta_y;
//     SwipeOperation(int dx = 0, int dy = 0) : delta_x(dx), delta_y(dy) {}
// };

struct PlayOperation {
    PlayOperationType type;
    union {
        ClickOperation click;
        int sleep_ms;
    };

    PlayOperation(PlayOperationType type) { this->type = type; }
};

class IPlayer {
public:
    virtual ~IPlayer() {}

    virtual bool Init() { return true; };

    virtual std::vector<PlayOperation>
    Play(const std::vector<ObjectBox> &object_boxes,
         const std::vector<TextBox> &text_boxes) = 0;

    virtual bool IsGameOver() { return false; }
};
