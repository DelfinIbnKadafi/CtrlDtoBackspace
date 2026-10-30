#include <windows.h>
#include <wchar.h>
#include "application.h"
#include "settings.h"

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int) {
  const wchar_t* cmd = GetCommandLineW();
  bool quit = wcsstr(cmd, L"--quit") != nullptr;
  bool minimized = wcsstr(cmd, L"--minimized") != nullptr;

  // --quit : close running app (installer use this)
  if(quit) {
    HWND old = FindWindowW(APP_WND_CLASS, nullptr);
    if(old) {
      PostMessageW(old, WM_CLOSE, 0, 0);
    }
    return 0;
  }

  // one instance only
  HANDLE mutex = CreateMutexW(nullptr, TRUE, APP_MUTEX_NAME);
  if(mutex == nullptr) {
    return 1;
  }
  if(GetLastError() == ERROR_ALREADY_EXISTS) {
    HWND old = FindWindowW(APP_WND_CLASS, nullptr);
    if(old && !minimized) {
      PostMessageW(old, WM_SHOW_SETTINGS, 0, 0);
    }
    CloseHandle(mutex);
    return 0;
  }

  if(!AppInit(inst, minimized)) {
    MessageBoxW(nullptr, L"Cannot start application.", APP_NAME, MB_OK | MB_ICONERROR);
    CloseHandle(mutex);
    return 1;
  }

  // normal message loop, sleep when no message
  MSG msg;
  while(GetMessageW(&msg, nullptr, 0, 0) > 0) {
    if(SettingsDialogMessage(&msg)) {
      continue;
    }
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  CloseHandle(mutex);
  return (int)msg.wParam;
}
