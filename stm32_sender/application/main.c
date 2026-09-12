#include "app.h"

int main(void)
{
    if (app_init() != STATUS_OK) {
        /* A failed early initialization intentionally stops here. If the IWDG
         * was already started, it will reset the board; otherwise a debugger
         * can inspect the failure path. */
        for (;;) {
            __asm volatile("nop");
        }
    }

    app_run();
    return 0;
}
