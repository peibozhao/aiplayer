#ifndef PLAYER_BLHX_PLAYER_H
#define PLAYER_BLHX_PLAYER_H

#include "player.h"

class BLHXPlayer : public IPlayer {
public:
  bool Init(const std::string &cfg) override;
  PlayOperation Play(const std::vector<DetectBox> &boxes) override;
};

#endif // ifndef PLAYER_BLHX_PLAYER_H

