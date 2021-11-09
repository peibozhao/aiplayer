
#include "application/blhx_application.h"
#include <iostream>
#include <thread>
#include <getopt.h>
#include <string.h>

struct CommandLine {
    std::string config;
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
        int ret = getopt_long(argc, argv, "m:c:", options.data(), NULL);
        if (ret == -1) {
            break;
        } else if (ret == '?') {
            std::cerr << "Unkown params" << std::endl;
            return false;
        } else if (ret == 'm') {
            cmd_line.mode = optarg;
        } else if (ret == 'c') {
            cmd_line.config = optarg;
        }
    }
    return true;
}

void RecvCommandThread(BlhxApplication *app) {
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
    if  (!ParseCommandLine(argc, argv)) {
        std::cerr << "Param error" << std::endl;
        return -1;
    }

    BlhxApplication app(cmd_line.config);
    if (!app.Init()) {
        std::cerr << "Application init failed" << std::endl;
        return -1;
    }
    if (!cmd_line.mode.empty()) {
        if (!app.SetParam("mode", cmd_line.mode)) {
            std::cerr << "SetParam failed" << std::endl;
            return -1;
        }
    }

    std::thread command_thread(&RecvCommandThread, &app);
    command_thread.detach();
    app.Run();
}
