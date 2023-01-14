/**
 * @brief Main entrypoint of the application
 * 
 */

#pragma warning(push, 0)

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#pragma warning(pop)

struct sln_app {
  HINSTANCE hinstance;
  HWND hwnd;
  uint32_t width;
  uint32_t height;
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
LRESULT win_message_proc(HWND window, UINT msg, WPARAM wparam,
  LPARAM lparam) {

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
  }

  return DefWindowProcA(window, msg, wparam, lparam);
}

DWORD win_game_loop(void *param) {

  struct sln_app *app = (struct sln_app *)param;

  uint64_t counter;
  uint64_t frequency;
  QueryPerformanceCounter((LARGE_INTEGER *)&counter);
  QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);

  uint32_t fps = 0;

  while (1) {
    sln_update(*app);

    uint64_t new_counter;
    QueryPerformanceCounter((LARGE_INTEGER *)&new_counter);

    fps++;
    if ((new_counter - counter) / frequency >= 1) {
      char buffer[64];

      sprintf_s(buffer, sizeof(buffer), "FPS: %u\n", fps);
      OutputDebugString(buffer);

      fps = 0;
      QueryPerformanceCounter((LARGE_INTEGER *)&counter);
    }
  }
}

#pragma warning(disable:4100)
/**
 * @brief Entrypoint for WIN64 builds
 * 
 * @param hinstance Instance of the program
 * @param prev_hinstance Deprecated
 * @param cmd Command line arguments
 * @param show Show mode (in exe properties)
 * @return int Return code
 */
int APIENTRY WinMain(HINSTANCE hinstance, HINSTANCE prev_hinstance, LPSTR cmd,
  int show) {
#pragma warning(default:4100)

  struct sln_app app = {0};
  app.hinstance = hinstance;

  // Register window class
  WNDCLASSEXA wc = {0};
  wc.cbSize = sizeof(wc);
  wc.hInstance = app.hinstance;
  wc.lpfnWndProc = win_message_proc;
  wc.lpszClassName = "12/01/2023Slinapp";

  RegisterClassExA(&wc);

  app.hwnd = CreateWindowExA(0, wc.lpszClassName, "App",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
    1920, 1080, 0, 0, hinstance, &app);

  sln_init(app);

  CreateThread(0, 0, win_game_loop, &app, 0, 0);

  while (1) {
    MSG msg;
    while (PeekMessageA(&msg, app.hwnd, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
    }

    // Not to overload window messages
    Sleep(10);
  }
}
