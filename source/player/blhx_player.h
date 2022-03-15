
#pragma once

#include "common_player.h"

class BlhxPlayer : public CommonPlayer {
public:
  BlhxPlayer(const std::string &name,
             const std::vector<PageConfig> &page_configs,
             const std::vector<ModeConfig> &mode_configs);

private:
  void RegisterSpecialPages() override;
};
