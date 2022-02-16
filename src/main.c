// #include "core/core.h"
#include "controller/controller.h"

int main(void) {
    // struct Game game;
    // core_initialize(&game, 1000);
    // for (int i = 0; i < 25; ++i) {
    //     core_draw(&game);
    // }
    // core_log(&game, stdout);
    controller_initialize();
    for (; !controller_handle(););
    controller_finalize();
    return 0;
}
