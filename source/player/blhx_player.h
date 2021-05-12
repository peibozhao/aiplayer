
/**
 * @file player.h
 * @brief
 * @author peibozhao
 * @version 1.0.0
 * @date 2020-10-13
 */

#pragma once

#include "player.h"
#include "player/blhx/base_model.h"
#include <string>
#include <vector>

class BLHXPlayer : public IPlayer {
public:
    BLHXPlayer();

    virtual ~BLHXPlayer() {}

    bool Init(const std::string &config_str) override;

    std::vector<PlayOperation> Play(const std::vector<ObjectBox> &objects,
                                    const std::vector<TextBox> &texts) override;

    bool GetLimit() override;

private:
    std::vector<IBLHXBaseMode *> submodes_;
    IBLHXBaseMode *cur_mode_;
};
