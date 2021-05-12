
#include "glog/logging.h"
#include "http_server.h"

char *GetCommandOptions(int argc, char *argv[], const std::string &flag) {
    char **end = argv + argc;
    char **begin = argv;
    auto iter = std::find(begin, end, flag);
    if (iter == end || iter + 1 == end) {
        return nullptr;
    }
    return *(iter + 1);
}

void Usage(char *cmd) {
    std::cout << "Usage: " << cmd << "--config FILE" << std::endl;
}

int main(int argc, char *argv[]) {
    char *config_fname = GetCommandOptions(argc, argv, "--config");
    if (config_fname == nullptr) {
        Usage(argv[0]);
        return -1;
    }

    HttpServer server;
    if (!server.Init(config_fname)) {
        LOG(ERROR) << "Server init failed";
        return -1;
    }
    server.Start();
    return 0;
}
