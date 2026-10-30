#include "application.h"
#include "keyboard_hook.h"
#include "tray.h"
#include "settings.h"
#include "startup.h"

static const wchar_t kRegKey[] = L"Software\\CtrlDtoBackspace";

static HWND g_wnd = nullptr;
static bool g_enabled = true;
static bool g_startMin = false;
static UINT g_msgTaskbar = 0;

static DWORD LoadDword(const wchar_t* name, DWORD def) {
  DWORD v = 0;
  DWORD size = sizeof(v);
  if(RegGetValueW(HKEY_CURRENT_USER, kRegKey, name, RRF_RT_REG_DWORD, nullptr, &v, &size) != ERROR_SUCCESS) {
    return def;
  }
  return v;
}

static void SaveDword(const wchar_t* name, DWORD v) {
  HKEY key;
  if(RegCreateKeyExW(HKEY_CURRENT_USER, kRegKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr) != ERROR_SUCCESS) {
    return;
  }
  RegSetValueExW(key, name, 0, REG_DWORD, (const BYTE*)&v, sizeof(v));
  RegCloseKey(key);
}

// hook on/off, no save here
static bool Apply(bool on) {
  if(on) {
    if(!KbHookInstall()) {
      return false;
    }
  } else {
    KbHookRemove();
  }
  g_enabled = on;
  TrayUpdate(on);
  SettingsRefresh();
  return true;
}

bool AppIsEnabled() {
  return g_enabled;
}

bool AppSetEnabled(bool on) {
  if(!Apply(on)) {
    MessageBoxW(g_wnd, L"Cannot install the keyboard hook.", APP_NAME, MB_OK | MB_ICONERROR);
    SettingsRefresh();
    return false;
  }
  SaveDword(L"Enabled", on ? 1 : 0);
  return true;
}

bool AppStartMinimized() {
  return g_startMin;
}

void AppSetStartMinimized(bool on) {
  g_startMin = on;
  SaveDword(L"StartMinimized", on ? 1 : 0);
}

void AppExit() {
  if(g_wnd) {
    PostMessageW(g_wnd, WM_CLOSE, 0, 0);
  }
}

static void ToggleStartup() {
  if(!StartupSet(!StartupIsEnabled())) {
    MessageBoxW(g_wnd, L"Cannot change startup setting.", APP_NAME, MB_OK | MB_ICONERROR);
  }
  SettingsRefresh();
}

static LRESULT CALLBACK WndProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
  // explorer restart, tray icon gone, add again
  if(g_msgTaskbar != 0 && msg == g_msgTaskbar) {
    TrayAdd(wnd, g_enabled);
    return 0;
  }

  switch(msg) {
    case WM_TRAY:
      if(lp == WM_RBUTTONUP) {
        TrayShowMenu(wnd);
      } else if(lp == WM_LBUTTONDBLCLK) {
        SettingsShow(wnd);
      }
      return 0;

    case WM_COMMAND:
      switch(LOWORD(wp)) {
        case IDM_ENABLE:
          AppSetEnabled(true);
          break;
        case IDM_DISABLE:
          AppSetEnabled(false);
          break;
        case IDM_SETTINGS:
          SettingsShow(wnd);
          break;
        case IDM_STARTUP:
          ToggleStartup();
          break;
        case IDM_EXIT:
          AppExit();
          break;
      }
      return 0;

    case WM_SHOW_SETTINGS:
      SettingsShow(wnd);
      return 0;

    case WM_CLOSE:
      DestroyWindow(wnd);
      return 0;

    case WM_DESTROY:
      // remove hook first, keyboard back to normal
      KbHookRemove();
      TrayRemove();
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProcW(wnd, msg, wp, lp);
}

bool AppInit(HINSTANCE inst, bool forceMinimized) {
  WNDCLASSEXW wc = {};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = WndProc;
  wc.hInstance = inst;
  wc.lpszClassName = APP_WND_CLASS;
  if(!RegisterClassExW(&wc)) {
    return false;
  }

  // hidden window, only for messages (not message-only, need TaskbarCreated)
  g_wnd = CreateWindowExW(0, APP_WND_CLASS, APP_NAME, WS_OVERLAPPED, 0, 0, 0, 0, nullptr, nullptr, inst, nullptr);
  if(!g_wnd) {
    return false;
  }

  g_msgTaskbar = RegisterWindowMessageW(L"TaskbarCreated");
  g_enabled = LoadDword(L"Enabled", 1) != 0;
  g_startMin = LoadDword(L"StartMinimized", 0) != 0;

  // if exe moved, fix the run key path
  if(StartupIsEnabled()) {
    StartupSet(true);
  }

  if(g_enabled && !KbHookInstall()) {
    g_enabled = false;
    MessageBoxW(g_wnd, L"Cannot install the keyboard hook.\nApp start as disabled.", APP_NAME, MB_OK | MB_ICONERROR);
  }

  TrayAdd(g_wnd, g_enabled);

  if(!forceMinimized && !g_startMin) {
    SettingsShow(g_wnd);
  }
  return true;
}
