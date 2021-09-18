
#include "device_operation/minitouch_operation.h"
#include <string>
#include <iostream>

void Usage() {
    std::cout << "./click X Y" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        Usage();
        return -1;
    }

    int x = std::stoi(argv[1]);
    int y = std::stoi(argv[2]);

    MinitouchOperation minitouch(1111);
    minitouch.Init();
    int id = minitouch.TouchDown(x, y);
    minitouch.TouchUp(id);
}
