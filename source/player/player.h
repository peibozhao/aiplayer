#pragma once

#include <vector>
#include <string>

struct Element {
    std::string name;
    float x, y;
};

enum class PlayOperationType {
    SCREEN_CLICK = 1, // 触屏点击操作
    SLEEP,            // 等待
    LIMITS            // 到达限制
};

struct ClickOperation {
    uint16_t x, y;
    ClickOperation(uint16_t x_center = 0, uint16_t y_center = 0)
        : x(x_center), y(y_center) {}
};

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

    IPlayer(const std::string &name) {
        name_ = name;
    }

    virtual bool Init() { return true; };

    /// @brief 通过检测结果返回行为
    virtual std::vector<PlayOperation>
    Play(const std::vector<Element> elements) = 0;

    virtual std::string Name() { return name_; }

    /// @brief 当前是否达到限制
    virtual bool GameOver() { return false; }

    /// @brief 到达限制后继续
    virtual void GameContinue() {}

    /// @brief 设置mode
    virtual bool SetMode(const std::string &mode) { return false; }

    /// @brief 当前mode
    virtual std::string GetMode() { return ""; }

private:
    std::string name_;
};
