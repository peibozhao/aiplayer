
#pragma once

#include "base_model.h"

class BattleMode : public IBLHXBaseMode {
public:
    std::vector<PlayOperation>
    MainPlay(const std::vector<ObjectBox> &objects,
             const std::vector<TextBox> &texts) override {
        return {Click(texts, "特别作")};  // TODO
    }

    std::vector<PlayOperation>
    WeighAnchorPlay(const std::vector<ObjectBox> &objects,
                    const std::vector<TextBox> &texts) override {
        return {Click(texts, "D3时此刻")};  // TODO
    }

    std::vector<PlayOperation>
    SpecialPlay(const std::vector<ObjectBox> &objects,
                const std::vector<TextBox> &texts) override {
        return {Click(texts, "D3时此刻")};  // TODO
    }

    std::vector<PlayOperation>
    AgainPlay(const std::vector<ObjectBox> &objects,
              const std::vector<TextBox> &texts) override {
        return {Click(texts, "再次前往")};
    }
};
