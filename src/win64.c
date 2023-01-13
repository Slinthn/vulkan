/**
 * @brief Main entrypoint of the application
 * 
 */

#define APP_WIN64

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
LRESULT window_message_proc(HWND window, UINT msg, WPARAM wparam,
  LPARAM lparam) {

  switch (msg) {
  case WM_CLOSE:
  case WM_DESTROY: {
    ExitProcess(0);
  } break;
  }

  return DefWindowProcA(window, msg, wparam, lparam);
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
  wc.lpfnWndProc = window_message_proc;
  wc.lpszClassName = "12/01/2023Slinapp";

  RegisterClassExA(&wc);

  app.hwnd = CreateWindowExA(0, wc.lpszClassName, "App",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hinstance, 0);

  uint64_t counter;
  uint64_t frequency;
  QueryPerformanceCounter((LARGE_INTEGER *)&counter);
  QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);

  sln_init(app);

  while (1) {
    MSG msg;
    while (PeekMessageA(&msg, app.hwnd, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
    }

    sln_update();

#if 0
    int32_t tosleep;
    do {
      uint64_t newcounter;
      QueryPerformanceCounter((LARGE_INTEGER *)&newcounter);
      float delta_seconds = ((newcounter - counter) / (float)frequency);
      tosleep = (int32_t)floorf((1 / 60.0f - delta_seconds) * 1000.0f);
    } while (tosleep > 0);
#endif

    QueryPerformanceCounter((LARGE_INTEGER *)&counter);
  }
}
