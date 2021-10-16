
#include "application/blhx_application.h"
#include <iostream>
#include <thread>

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
    if  (argc < 2) {
        std::cerr << "Param error" << std::endl;
        return -1;
    }
    BlhxApplication app(argv[1]);
    if (!app.Init()) {
        std::cerr << "Application init failed" << std::endl;
        return -1;
    }

    std::thread command_thread(&RecvCommandThread, &app);
    app.Run();
}
