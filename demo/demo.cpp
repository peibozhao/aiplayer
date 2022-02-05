
#include "application/common_application.h"
#include <getopt.h>
#include <iostream>
#include <string.h>
#include <thread>

struct CommandLine {
    std::string config;
    std::string player;
    std::string mode;
};

CommandLine cmd_line;

bool ParseCommandLine(int argc, char *argv[]) {
    std::vector<option> options;

    struct option config_opt;
    memset(&config_opt, 0, sizeof(config_opt));
    config_opt.name = "config";
    config_opt.has_arg = required_argument;
    config_opt.flag = NULL;
    config_opt.val = 'c';
    options.push_back(config_opt);

    struct option player_opt;
    memset(&player_opt, 0, sizeof(player_opt));
    player_opt.name = "player";
    player_opt.has_arg = required_argument;
    player_opt.flag = NULL;
    player_opt.val = 'p';
    options.push_back(player_opt);

    struct option mode_opt;
    memset(&mode_opt, 0, sizeof(mode_opt));
    mode_opt.name = "mode";
    mode_opt.has_arg = required_argument;
    mode_opt.flag = NULL;
    mode_opt.val = 'm';
    options.push_back(mode_opt);

    struct option end_opt;
    memset(&end_opt, 0, sizeof(end_opt));
    options.push_back(end_opt);

    while (true) {
        int ret = getopt_long(argc, argv, "c:p:m:", options.data(), NULL);
        if (ret == -1) {
            break;
        } else if (ret == '?') {
            std::cerr << "Unkown params" << std::endl;
            return false;
        } else if (ret == 'p') {
            cmd_line.player = optarg;
        } else if (ret == 'm') {
            cmd_line.mode = optarg;
        } else if (ret == 'c') {
            cmd_line.config = optarg;
        }
    }
    return true;
}

void RecvCommandThread(CommonApplication *app) {
    std::string command;
    while (true) {
        std::cin >> command;

        if (command == "pause" || command == "p") {
            app->Pause();
        } else if (command == "continue" || command == "c") {
            app->Continue();
        } else {
            std::cerr << "Unknown command: " << command.c_str() << std::endl;
        }
    }
}

int main(int argc, char *argv[]) {
    if (!ParseCommandLine(argc, argv)) {
        std::cerr << "Param error" << std::endl;
        return -1;
    }

    CommonApplication app(cmd_line.config);
    if (!app.Init()) {
        std::cerr << "Application init failed" << std::endl;
        return -1;
    }
    if (!cmd_line.player.empty() && !app.SetPlayer(cmd_line.player)) {
        std::cerr << "Set player failed" << std::endl;
        return -1;
    }
    if (!cmd_line.mode.empty() && !app.SetMode(cmd_line.mode)) {
        std::cerr << "Set mode failed" << std::endl;
        return -1;
    }

    std::thread command_thread(&RecvCommandThread, &app);
    command_thread.detach();
    app.Start();
}
