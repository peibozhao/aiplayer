
#include "http_server.h"

int main(int argc, char *argv[]) {
    BlhxHttpServer server;
    server.InitWithFile("server.yaml");
    server.Start();
}
