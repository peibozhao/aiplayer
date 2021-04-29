
#include "battle_player.h"
#include "yaml-cpp/yaml.h"
#include <random>

#define _PLAY_CENTER(obj, prio)                                                                    \
    if (priority < prio) {                                                                         \
        ret = {CreatePlayOperation(PlayOperationType::SCREEN_CLICK, Pair2Vector(BoxCenter(obj)))}; \
        priority = prio;                                                                           \
    }

static std::pair<int, int> BoxCenter(const ObjectBox &box) {
    return std::make_pair(box.x + box.width / 2, box.y + box.height / 2);
}

static std::pair<int, int> BoxCenter(const TextBox &box) {
    return std::make_pair(box.x + box.width / 2, box.y + box.height / 2);
}

static PlayOperation CreatePlayOperation(PlayOperationType type, std::vector<int> num) {
    PlayOperation ret;
    ret.type = type;
    switch (type) {
    case PlayOperationType::SLEEP:
        ret.sleep.time = num[0];
        break;
    case PlayOperationType::SCREEN_CLICK:
        ret.click.x = num[0];
        ret.click.y = num[1];
        break;
    case PlayOperationType::SCREEN_SWIPE:
        ret.swipe.delta_x = num[0];
        ret.swipe.delta_y = num[1];
        break;
    default:
        ret.type = PlayOperationType::NONE;
        break;
    }
    return ret;
}

static std::vector<int> Pair2Vector(std::pair<int, int> pair) { return {pair.first, pair.second}; }

bool BattlePlayer::Init(std::istream &is) {
    try {
        YAML::Node config = YAML::Load(is);
        const YAML::Node &screen = config["screen"];
        width_ = screen["width"].as<int>();
        height_ = screen["height"].as<int>();
        chapter_pattern_ = config["chapter_pattern"].as<std::string>(),
        name_pattern_ = config["name_pattern"].as<std::string>(),
        left_times_ = config["times"].as<int>();
    } catch (std::exception &e) {
        SPDLOG_ERROR("Catch exception. {}", e.what());
        return false;
    }
    return true;
}

std::vector<PlayOperation> BattlePlayer::Play(const std::vector<ObjectBox> &object_boxes,
                                              const std::vector<TextBox> &text_boxes) {
    for (auto &object_box : object_boxes) {
        if (!boss_appeared_ && object_box.name == "boss") {
            for (auto &text_box : text_boxes) {
                if (text_box.text == "切换") {
                    SPDLOG_DEBUG("Change team");
                    boss_appeared_ = true;
                    return {CreatePlayOperation(PlayOperationType::SCREEN_CLICK,
                                                Pair2Vector(BoxCenter(object_box)))};
                }
            }
            SPDLOG_WARN("boss appeard, change not appeared");
            return {};
        }
    }

    std::vector<PlayOperation> ret;
    int priority = -1;
    for (const auto &text_box : text_boxes) {
        if (Match(text_box.text, chapter_pattern_)) {
            _PLAY_CENTER(text_box, 9)
        } else if (Match(text_box.text, "点击继续")) {
            _PLAY_CENTER(text_box, 10)
        } else if (Match(text_box.text, "点击关闭")) {
            SPDLOG_WARN("Defeat");
            _PLAY_CENTER(text_box, 10)
        } else if (Match(text_box.text, "锁定") || Match(text_box.text, "分享") ||
                   Match(text_box.text, "检视")) {
            if (priority < 10) {
                ret = {CreatePlayOperation(PlayOperationType::SCREEN_CLICK, {1000, 500})};
                priority = 10;
            }
        } else if (Match(text_box.text, "立刻前往")) {
            Reset();
            _PLAY_CENTER(text_box, 10)
        } else if (Match(text_box.text, "点击继续")) {
            _PLAY_CENTER(text_box, 10)
        } else if (Match(text_box.text, "确定")) {
            _PLAY_CENTER(text_box, 10)
        } else if (Match(text_box.text, "出击")) {
            _PLAY_CENTER(text_box, 10)
        } else if (Match(text_box.text, name_pattern_)) {
            _PLAY_CENTER(text_box, 1)
        } else if (Match(text_box.text, "撤退") || Match(text_box.text, "切换")) {
            // Battle
            if (priority < 10) {
                ret = AttackEnemy(object_boxes);
                priority = 10;
            }
        }
    }
    return {ret};
}

bool BattlePlayer::GetLimit() { return left_times_ <= 0; }

std::vector<PlayOperation> BattlePlayer::AttackEnemy(const std::vector<ObjectBox> &object_boxes) {
    std::string name = boss_appeared_ ? "boss" : "敌人";
    for (auto &object_box : object_boxes) {
        if (object_box.name == name) {
            return Move(object_box);
        }
    }
    int len = 1000;
    std::mt19937 rand_engine(time(0));
    std::uniform_real_distribution<float> rand_dis(0, 3.14 * 2);
    float radian = rand_dis(rand_engine);
    return {CreatePlayOperation(PlayOperationType::SCREEN_SWIPE,
                                {int(len * std::sin(radian)), int(len * std::cos(radian))})};
}

std::vector<PlayOperation> BattlePlayer::Move(const ObjectBox &object_box) {
    return {
        CreatePlayOperation(PlayOperationType::SCREEN_CLICK, Pair2Vector(BoxCenter(object_box)))};
}

void BattlePlayer::Reset() {
    SPDLOG_DEBUG("Battle left {}", left_times_);
    left_times_ -= 1;
    boss_appeared_ = false;
}

PlayOperation BattlePlayer::CheckBoundray(const PlayOperation &opt) {
    if (opt.type == PlayOperationType::SCREEN_CLICK &&
        (opt.click.x < boundary_width_ || opt.click.x > width_ - boundary_width_ ||
         opt.click.y < boundary_width_ || opt.click.y > height_ - boundary_width_)) {
        SPDLOG_DEBUG("Click boundary: {} {}", opt.click.x, opt.click.y);
        PlayOperation ret;
        ret.type = PlayOperationType::SCREEN_SWIPE;
        ret.swipe.delta_x = width_ / 2 - opt.click.x;
        ret.swipe.delta_y = height_ / 2 - opt.click.y;
        return ret;
    }
    return opt;
}

bool BattlePlayer::Match(const std::string &str, const std::string &pattern) {
    return std::regex_match(str, std::regex(pattern));
}
