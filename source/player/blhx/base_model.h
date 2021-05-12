
#pragma once

#include "player/player.h"

#define DeclarePage(page_name, ...)                                            \
    bool Is##page_name(const std::vector<ObjectBox> &objects,                  \
                       const std::vector<TextBox> &texts) {                    \
        for (const std::string &name : {__VA_ARGS__}) {                        \
            bool has = false;                                                  \
            for (const ObjectBox &object : objects) {                          \
                if (object.name == name) {                                     \
                    has = true;                                                \
                    break;                                                     \
                }                                                              \
            }                                                                  \
            if (has) {                                                         \
                continue;                                                      \
            }                                                                  \
            for (const TextBox &text : texts) {                                \
                if (text.text == name) {                                       \
                    has = true;                                                \
                    break;                                                     \
                }                                                              \
            }                                                                  \
            if (!has) {                                                        \
                return false;                                                  \
            }                                                                  \
        }                                                                      \
        return true;                                                           \
    }                                                                          \
    virtual std::vector<PlayOperation> page_name##Play(                        \
        const std::vector<ObjectBox> &objects,                                 \
        const std::vector<TextBox> &texts) {                                   \
        return {};                                                             \
    }

class IBLHXBaseMode {
public:
    // 主页
    DeclarePage(Main, "出击", "建造");
    // 主页 出击
    DeclarePage(WeighAnchor, "主线", "大型作战");
    // 主页 出击 演习
    DeclarePage(Operation, "规则说明", "排行榜", "军需补给");
    // 主页 出击 主线
    DeclarePage(WeighAnchorMain, "限界挑战", "每日任务", "委托任务", "演习");
    // 主页 出击 主线 章节
    DeclarePage(WeighAnchorMainStage, "自律寻敌", "立刻前往");
    // 主页 出击 主线 章节 立刻前往
    DeclarePage(FleetSelect, "舰队选择", "立刻前往");
    // 主页 出击 主线 章节 立刻前往 立刻前往
    DeclarePage(SubChapter, "撤退", "切换", "迎击");
    // 被击败
    DeclarePage(Defeat, "提高等级", "装备强化", "技能提升", "突破强化");
    // 再次前往
    DeclarePage(Again, "离开", "再次前往");
};
