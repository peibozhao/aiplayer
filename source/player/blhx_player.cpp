
#include "blhx_player.h"
#include "player/blhx/yanxi_mode.h"
#include "player/blhx/battle_mode.h"

BLHXPlayer::BLHXPlayer() {
    IBLHXBaseMode *yanxi_mode = new YanxiMode();
    IBLHXBaseMode *battle_mode = new BattleMode();
    submodes_.push_back(battle_mode);
    cur_mode_ = battle_mode;
}

bool BLHXPlayer::Init(const std::string &config_str) {
    // for (IBLHXBaseMode *mode : submodes_) {
    //     if (!mode->Init(config_str)) {
    //         return false;
    //     }
    // }
    return true;
}

std::vector<PlayOperation> BLHXPlayer::Play(const std::vector<ObjectBox> &objects,
                                            const std::vector<TextBox> &texts) {

    if (cur_mode_->IsMain(objects, texts)) {
        return cur_mode_->MainPlay(objects, texts);
    } else if (cur_mode_->IsWeighAnchor(objects, texts)) {
        return cur_mode_->WeighAnchorPlay(objects, texts);
    } else if (cur_mode_->IsOperation(objects, texts)) {
        return cur_mode_->OperationPlay(objects, texts);
    } else if (cur_mode_->IsStartOperation(objects, texts)) {
        return cur_mode_->StartOperationPlay(objects, texts);
    } else if (cur_mode_->IsWeighAnchorMain(objects, texts)) {
        return cur_mode_->WeighAnchorMainPlay(objects, texts);
    } else if (cur_mode_->IsImmediateStart(objects, texts)) {
        return cur_mode_->ImmediateStartPlay(objects, texts);
    } else if (cur_mode_->IsSubChapter(objects, texts)) {
        return cur_mode_->SubChapterPlay(objects, texts);
    } else if (cur_mode_->IsSpecial(objects, texts)) {
        return cur_mode_->SpecialPlay(objects, texts);
    } else if (cur_mode_->IsDefeat(objects, texts)) {
        return cur_mode_->DefeatPlay(objects, texts);
    } else if (cur_mode_->IsAgain(objects, texts)) {
        return cur_mode_->AgainPlay(objects, texts);
    } else if (cur_mode_->IsCheckpointTask(objects, texts)) {
        return cur_mode_->CheckpointTaskPlay(objects, texts);
    } else if (cur_mode_->IsGetItem(objects, texts)) {
        return cur_mode_->GetItemPlay(objects, texts);
    } else if (cur_mode_->IsOK(objects, texts)) {
        return cur_mode_->OKPlay(objects, texts);
    } else if (cur_mode_->IsFormation(objects, texts)) {
        return cur_mode_->FormationPlay(objects, texts);
    }
    return {};
}
bool BLHXPlayer::GetLimit() { return false; }

