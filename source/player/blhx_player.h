
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
#include "player.h"
#include "utils/util_defines.h"

class BLHXPlayer : public IPlayer {
public:
    virtual ~BLHXPlayer() {}

    bool Init(const std::string &config_str) override;

    std::vector<PlayOperation> Play(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) override;

    bool GetLimit() override { return false; }

private:
    enum Page {
    };

protected:
    virtual std::vector<PlayOperation> WeighAnchorPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> SubWeighAnchorPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> ImmediateStartPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> FleetSelectPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> SubChapterPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> FormationPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> PausePlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> FightingPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> CheckpointPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> GetItemsPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> MvpPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> InfomationPlay(const std::vector<ObjectBox> &object, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> DailyraidPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> SubDailyraidPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> AttackPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual std::vector<PlayOperation> DefaultPlay(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }

    virtual bool IsWeighAnchor(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsSubWeighAnchor(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsImmediateStart(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsFleetSelect(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsSubChapter(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsFormation(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsPause(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsFighting(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsCheckpoint(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsGetItems(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsMvp(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsInfomation(const std::vector<ObjectBox> &object, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsDailyraid(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsSubDailyraid(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsAttack(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
    virtual bool IsDefault(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) { return {}; }
};

