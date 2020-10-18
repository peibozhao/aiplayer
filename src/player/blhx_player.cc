
#include "blhx_player.h"
#include <regex>

bool BLHXPlayer::Init(const std::string &cfg) {
  return true;
}

PlayOperation BLHXPlayer::Play(const std::vector<DetectBox> &boxes) {
  PlayOperation ret;
  ret.type = PlayOperationType::SCREEN_CLICK;
  ret.click.x = 1000;
  ret.click.y = 10;
  for (const auto &box : boxes) {
    std::smatch matchs;
    if (std::regex_search(box.class_name, matchs, std::regex("label"))) {
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
  return ret;
}

