
#pragma once

#include "base_model.h"

class YanxiMode : public IBLHXBaseMode {
public:
    std::vector<PlayOperation>
    MainPlay(const std::vector<ObjectBox> &objects,
             const std::vector<TextBox> &texts) override {
        return {Click(texts, "出击")};
    }

    std::vector<PlayOperation>
    WeighAnchorPlay(const std::vector<ObjectBox> &objects,
                    const std::vector<TextBox> &texts) override {
        return {Click(texts, "演习")};
    }

    std::vector<PlayOperation>
    OperationPlay(const std::vector<ObjectBox> &objects,
                  const std::vector<TextBox> &texts) override {
        PlayOperation opt;
        opt.type = PlayOperationType::SCREEN_CLICK;
        opt.click.x = 500;
        opt.click.y = 300;
        return {opt};
    }

    std::vector<PlayOperation>
    StartOperationPlay(const std::vector<ObjectBox> &objects,
                       const std::vector<TextBox> &texts) override {
        return {Click(texts, "开演习")};
    }
};
