/**
 * @brief Main entrypoint of the application
 * 
 */

#pragma warning(push, 0)

#include <windows.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#pragma warning(pop)

// TODO create its own file
#define SIZEOF_ARRAY(x) (sizeof(x) / sizeof((x)[0]))

#include "vulkan.c"

struct win64_state {
  uint64_t unused;
};

LRESULT window_message_proc(HWND window, UINT msg, WPARAM wparam,
  LPARAM lparam) {

#if 0
  struct win64_state *state =
    (struct win64_state *)GetWindowLongPtrA(window, GWLP_USERDATA);
#endif

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
  }

  return DefWindowProcA(window, msg, wparam, lparam);
}

int APIENTRY WinMain(HINSTANCE hinstance, HINSTANCE prev_hinstance, LPSTR cmd,
  int show) {

  // TODO: Suppress unused parameter warnings
  (void)prev_hinstance;
  (void)cmd;
  (void)show;

  // Register window class
  WNDCLASSEXA wc = {0};
  wc.cbSize = sizeof(wc);
  wc.hInstance = hinstance;
  wc.lpfnWndProc = window_message_proc;
  wc.lpszClassName = "12/01/2023Slinapp";

  RegisterClassExA(&wc);

  struct win64_state winstate = {0};

  HWND hwnd = CreateWindowExA(0, wc.lpszClassName, "App",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hinstance, &winstate);

  vk_init(hinstance, hwnd);

  uint64_t counter;
  uint64_t frequency;
  QueryPerformanceCounter((LARGE_INTEGER *)&counter);
  QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);

  while (1) {
    MSG msg;
    while (PeekMessageA(&msg, hwnd, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageA(&msg);
    }

    int32_t tosleep;
    do {
      uint64_t newcounter;
      QueryPerformanceCounter((LARGE_INTEGER *)&newcounter);
      float delta_seconds = ((newcounter - counter) / (float)frequency);
      tosleep = (int32_t)floorf((1 / 60.0f - delta_seconds) * 1000.0f);
    } while (tosleep > 0);

    QueryPerformanceCounter((LARGE_INTEGER *)&counter);
  }
}
