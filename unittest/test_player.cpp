
#include "catch2/catch.hpp"
#include "player/blhx_player.h"

TEST_CASE("Player") {
  SECTION("BLHX") {
    BLHXPlayer player;
    player.Init("");
    DetectObject obj1, obj2;
    obj1.xmin = 300; obj2.xmin = 30;
    obj1.ymin = 400; obj2.ymin = 50;
    obj1.xmax = 500; obj2.xmax = 80;
    obj1.ymax = 800; obj2.ymax = 70;
    obj1.conf = 0.93; obj2.conf = 0.89;
    obj1.name = "enemy-normal";
    obj2.name = "enemy-normal";
    std::vector<DetectObject> objes;
    objes.emplace_back(obj1);
    objes.emplace_back(obj2);

    PlayOperation opt = player.Play(objes);
    CHECK(opt.type == PlayOperationType::SCREEN_CLICK);
    CHECK(opt.click.x == 400);
    CHECK(opt.click.y == 600);
  }
}
