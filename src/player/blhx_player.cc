
#include "blhx_player.h"
#include <regex>
#include "utils/util_types.h"

bool BLHXPlayer::Init(const std::string &cfg) {
  normal_enemy_times_ = 0;
  return true;
}

PlayOperation BLHXPlayer::Play(const std::vector<DetectBox> &boxes) {
  TimeLog time_log("Player");
  bool is_normal_enemy_ = true;
  PlayOperation ret;
  ret.type = PlayOperationType::SCREEN_CLICK;
  ret.click.x = 1000;
  ret.click.y = 10;
  for (const auto &box : boxes) {
    std::smatch matchs;
    if (std::regex_search(box.class_name, matchs, std::regex("label"))) {
      is_normal_enemy_ = false;
      // label
      if (std::regex_search(box.class_name, matchs, std::regex("meirirenwu"))) {
        // meirirenwu
        ret.type = PlayOperationType::SCREEN_CLICK;
        ret.click.x = 1450;
        ret.click.y = 650;
        break;
      } else {
        // likeqianwang, yingji, chuji, zhandoupingjia, dianjijixu, queding
        ret.type = PlayOperationType::SCREEN_CLICK;
        ret.click.x = (box.xmin + box.xmax) / 2;
        ret.click.y = (box.ymin + box.ymax) / 2;
        break;
      }
    } else if (std::regex_search(box.class_name, matchs, std::regex("enemy"))) {
      // enemy
      if (std::regex_search(box.class_name, matchs, std::regex("meirirenwu"))) {
        // boss
        is_normal_enemy_ = false;
        ret.type = PlayOperationType::SCREEN_CLICK;
        ret.click.x = (box.xmin + box.xmax) / 2;
        ret.click.y = (box.ymin + box.ymax) / 2;
        break;
      } else {
        // normal
        ret.type = PlayOperationType::SCREEN_CLICK;
        ret.click.x = (box.xmin + box.xmax) / 2;
        ret.click.y = (box.ymin + box.ymax) / 2;
      }
    }
  }
  // 防止boss时没有弹药导致失败
  normal_enemy_times_ = is_normal_enemy_ ? normal_enemy_times_ + 1 : 0;
  if (normal_enemy_times_ >= 5) {
    normal_enemy_times_ = 4;
    ret.type = PlayOperationType::NONE;
  }
  spdlog::debug("Play {} {} {}", ret.type, ret.click.x, ret.click.y);
  return ret;
}

