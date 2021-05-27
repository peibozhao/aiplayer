
#pragma once

#include "base_model.h"

class YanxiMode : public IBLHXBaseMode {
public:
    std::vector<PlayOperation>
    MainPlay(const std::vector<ObjectBox> &objects,
             const std::vector<TextBox> &texts) override;

    std::vector<PlayOperation>
    WeighAnchorPlay(const std::vector<ObjectBox> &objects,
                    const std::vector<TextBox> &texts) override;

    std::vector<PlayOperation>
    OperationPlay(const std::vector<ObjectBox> &objects,
                  const std::vector<TextBox> &texts) override;

    std::vector<PlayOperation>
    StartOperationPlay(const std::vector<ObjectBox> &objects,
                       const std::vector<TextBox> &texts) override;
};
