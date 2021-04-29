
#include "blhx_player.h"

std::vector<PlayOperation> BLHXPlayer::Play(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) {
    Page page = RecognizePage(objects, texts);
    switch (page) {
        case Unkown:
            return {};
        case Capter:
            return CapterPlay(objects, texts);
        case Start:
            return CapterPlay(objects, texts);
        case FleetSelect:
            return CapterPlay(objects, texts);
        case SubChapter:
            return CapterPlay(objects, texts);
        case Formation:
            return CapterPlay(objects, texts);
        case Pause:
            return CapterPlay(objects, texts);
        case Fighting:
            return CapterPlay(objects, texts);
        case Checkpoint:
            return CapterPlay(objects, texts);
        case GetItems:
            return CapterPlay(objects, texts);
        case Mvp:
            return CapterPlay(objects, texts);
        case DaiyTasks:
            return CapterPlay(objects, texts);
        case SubDailyTask:
            return CapterPlay(objects, texts);
        case Attack:
            return CapterPlay(objects, texts);
        default:
            return {};
    }
}

BLHXPlayer::Page BLHXPlayer::RecognizePage(const std::vector<ObjectBox> &objects, const std::vector<TextBox> &texts) {
    return Page::Unkown;
}
