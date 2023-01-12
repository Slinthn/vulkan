/**
 * @brief Main entrypoint of the application
 * 
 */

#pragma warning(push, 0)

#include <windows.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
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

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmd,
  int show) {

  // Suppress unused parameter warnings
  (void)prevInstance;
  (void)cmd;
  (void)show;

  // Register window class
  WNDCLASSEXA wc = {0};
  wc.cbSize = sizeof(wc);
  wc.hInstance = instance;
  wc.lpfnWndProc = window_message_proc;
  wc.lpszClassName = "12/01/2023Slinapp";

  RegisterClassExA(&wc);

  struct win64_state winstate = {0};

  HWND window = CreateWindowExA(0, wc.lpszClassName, "App",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, &winstate);

  vk_init();

  uint64_t counter;
  uint64_t frequency;
  QueryPerformanceCounter((LARGE_INTEGER *)&counter);
  QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);

  while (1) {
    MSG msg;
    while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE)) {
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
