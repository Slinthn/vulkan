#pragma warning(push, 0)
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <hidusage.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#pragma warning(pop)

#define FATAL_ERROR(x) { \
    MessageBoxExA(0, x, "FATAL ERROR", MB_OK | MB_ICONERROR, 0); \
    ExitProcess(1); }

#include "macros.c"
#include "math/math.h"
#include "rawinput/rawinput.h"

struct user_controls {  // TODO: move somewhere else
  union vector2 move;
  union vector2 look;
  uint64_t actions;
  uint64_t is_controller;
  uint8_t keypress[4];
  uint32_t unused0;
};

#include "game.c"

/**
 * @brief Window message handler. Called when something happens to the window
 * 
 * @param window Window affected
 * @param msg Message type
 * @param wparam First parameter
 * @param lparam Second parameter
 * @return LRESULT Return code
 */
LRESULT win_message_proc(
    HWND window,
    UINT msg,
    WPARAM wparam,
    LPARAM lparam
){
    struct sln_app *state =
        (struct sln_app *)GetWindowLongPtrA(window, GWLP_USERDATA);

    switch (msg) {
    case WM_CREATE: {
        // Set the windows state variable pointer as userdata in the window
        CREATESTRUCT *createstruct = (CREATESTRUCT *)lparam;
        SetWindowLongPtrA(window, GWLP_USERDATA,
            (LONG_PTR)createstruct->lpCreateParams);

        return 1;
    }

    case WM_CLOSE:
    case WM_DESTROY: {
        ExitProcess(0);
    } break;

    case WM_SIZE: {
        state->width = LOWORD(lparam);
        state->height = HIWORD(lparam);
    } break;

    case WM_INPUT: {
        rawinput_parse(&state->controls, (HRAWINPUT)lparam);
    } break;
    }

    return DefWindowProcA(window, msg, wparam, lparam);
}

/**
 * @brief Game loop. Should be on a separate thread
 * 
 * @param param Pointer to a sln_app structure
 * @return DWORD Return code
 */
DWORD win_game_loop(
    void *param
){
    struct sln_app *app = (struct sln_app *)param;

    uint64_t counter;
    uint64_t frequency;
    QueryPerformanceCounter((LARGE_INTEGER *)&counter);
    QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);

    uint32_t fps = 0;

    while (1) {
        sln_update(app);

        uint64_t new_counter;
        QueryPerformanceCounter((LARGE_INTEGER *)&new_counter);

        fps++;

#ifdef SLN_DEBUG
        if ((new_counter - counter) / frequency >= 1) {
            char buffer[64];

            sprintf_s(buffer, sizeof(buffer), "FPS: %u\n", fps);
            OutputDebugString(buffer);

            fps = 0;
            QueryPerformanceCounter((LARGE_INTEGER *)&counter);
        }
#endif  // SLN_DEBUG
    }
}


/**
 * @brief Entrypoint for WIN64 builds
 * 
 * @param hinstance Instance of the program
 * @param prev_hinstance Deprecated
 * @param cmd Command line arguments
 * @param show Show mode (in exe properties)
 * @return int Return code
 */
int APIENTRY WinMain(
    HINSTANCE hinstance,
    HINSTANCE prev_hinstance,
    LPSTR cmd,
    int show
){
    // Suppress warnings
    (void)prev_hinstance;
    (void)cmd;
    (void)show;

    struct sln_app app = {0};
    struct graphics_surface surface = {0};
    surface.hinstance = hinstance;

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(wc);
    wc.hInstance = hinstance;
    wc.lpfnWndProc = win_message_proc;
    wc.lpszClassName = "12/01/2023Slinapp";

    RegisterClassExA(&wc);

    surface.hwnd = CreateWindowExA(0, wc.lpszClassName, "App",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        SLN_WINDOW_WIDTH, SLN_WINDOW_HEIGHT, 0, 0, hinstance, &app);

    rawinput_init(surface.hwnd);
    sln_init(&app, surface);

    CreateThread(0, 0, win_game_loop, &app, 0, 0);

    while (1) {
        MSG msg;
        while (PeekMessageA(&msg, surface.hwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        // Not to overload window messages
        Sleep(16);
    
        if (!app.controls.is_controller)
            app.controls.look = (union vector2){0};
    }
}
