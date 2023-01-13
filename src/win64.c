/**
 * @brief Main entrypoint of the application
 * 
 */

#define APP_WIN64

#pragma warning(push, 0)

#include <windows.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define VK_USE_PLATFORM_WIN32_KHR

#include <vulkan/vulkan.h>

#pragma warning(pop)

// TODO: create its own file
#define SIZEOF_ARRAY(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGN_UP(x, alignment) (((x) + ((alignment) - 1)) & ~((alignment) - 1))

struct win64_state {
  uint64_t unused;  // TODO: Unused
};

struct sln_app {
  HINSTANCE hinstance;
  HWND hwnd;
};

#include "graphics/vulkan.c"
#include "game.c"

#if 0
struct sln_file {
  void *data;
  uint64_t size;
};

DWORD sln_read_file(char *filename, struct sln_file *file) {

  HANDLE filehandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ,
    0, OPEN_EXISTING, 0, 0);

  if (filehandle == INVALID_HANDLE_VALUE)
    return GetLastError();  // TODO: Only in debug?

  GetFileSizeEx(filehandle, (PLARGE_INTEGER)&file->size);

  file->data = malloc(file->size);  // TODO: Better allocation?

  ReadFile(filehandle, file->data, file->size, 0, 0);

  return GetLastError();  // TODO: Only in debug?
}

void sln_close_file(void) {

}
#endif

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

  struct sln_app app = {0};
  app.hinstance = hinstance;

  // Register window class
  WNDCLASSEXA wc = {0};
  wc.cbSize = sizeof(wc);
  wc.hInstance = app.hinstance;
  wc.lpfnWndProc = window_message_proc;
  wc.lpszClassName = "12/01/2023Slinapp";

  RegisterClassExA(&wc);

  struct win64_state winstate = {0};

  app.hwnd = CreateWindowExA(0, wc.lpszClassName, "App",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hinstance, &winstate);

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
