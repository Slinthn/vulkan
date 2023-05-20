//#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#define OUT 

#define FATAL_ERROR(x) { \
    printf("FATAL ERROR: %s\n", x); \
    exit(1); }

#define max(x, y) MAX(x, y)
#define min(x, y) MIN(x, y)

#include "macros.c"
#include "math/math.h"

struct user_controls {  // TODO: move somewhere else
  union vector2 move;
  union vector2 look;
  uint64_t actions;
  uint64_t is_controller;
  uint8_t keypress[4];
  uint32_t unused0;
};

#include "game.c"

int main(
    void
){
    xcb_connection_t *connection = xcb_connect(0, 0);
    xcb_setup_t *setup = (xcb_setup_t *)xcb_get_setup(connection);
    xcb_window_t window = xcb_generate_id(connection);
    xcb_screen_iterator_t screens = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = screens.data;
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root,
        0, 0, 150, 150, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
        0, 0);
    xcb_map_window(connection, window);
    xcb_flush(connection);

    struct graphics_surface surface = {0};
    surface.connection = connection;
    surface.window = window;

    struct sln_app app = {0};
    app.width = 150;
    app.height = 150;
    sln_init(&app, surface);

    while (1) {
        sln_update(&app);
    }

    return 0;
}

