
#include "application/blhx_application.h"

int main(int argc, char *argv[]) {
    BlhxApplication app(argv[1]);
    if (!app.Init()) {
        return -1;
    }
    app.Run();
}
