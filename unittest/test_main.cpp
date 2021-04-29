
#define CATCH_CONFIG_RUNNER

#include "catch2/catch.hpp"
#include "spdlog/spdlog.h"

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    return Catch::Session().run(argc, argv);
}
