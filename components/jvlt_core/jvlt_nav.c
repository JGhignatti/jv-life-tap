#include "jvlt_internal.h"

jvlt_screen_t g_screen;

jvlt_screen_t jvlt_screen(void) { return g_screen; }

void jvlt_set_screen(jvlt_screen_t screen) {
    g_screen = screen;
    g_dirty = true;
}
