
#include "catch2/catch.hpp"
#include "player/blhx_player.h"

TEST_CASE("Player") {
  SECTION("BLHX") {
    BLHXPlayer player;
    player.Init("");
    DetectBox box1, box2;
    box1.xmin = 300; box2.xmin = 30;
    box1.ymin = 400; box2.ymin = 50;
    box1.xmax = 500; box2.xmax = 80;
    box1.ymax = 800; box2.ymax = 70;
    box1.conf = 0.93; box2.conf = 0.89;
    box1.class_idx = 1; box2.class_idx = 1;
    std::vector<DetectBox> boxes;
    boxes.emplace_back(box1);
    boxes.emplace_back(box2);

    PlayOperation opt = player.Play(boxes);
    CHECK(opt.type == PlayOperationType::SCREEN_CLICK);
    CHECK(opt.click.x == 400);
    CHECK(opt.click.y == 600);
  }
}
