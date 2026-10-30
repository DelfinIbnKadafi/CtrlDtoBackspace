#include "tray.h"
#include "application.h"
#include "startup.h"
#include "resource.h"
#include <shellapi.h>

static NOTIFYICONDATAW g_nid;
static bool g_added = false;

static HICON LoadTrayIcon(bool enabled) {
  int id = enabled ? IDI_APP : IDI_APP_OFF;
  return (HICON)LoadImageW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(id), IMAGE_ICON,
    GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
}

static void Fill(HWND wnd, bool enabled) {
  ZeroMemory(&g_nid, sizeof(g_nid));
  g_nid.cbSize = sizeof(g_nid);
  g_nid.hWnd = wnd;
  g_nid.uID = 1;
  g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  g_nid.uCallbackMessage = WM_TRAY;
  g_nid.hIcon = LoadTrayIcon(enabled);
  lstrcpyW(g_nid.szTip, enabled ? L"CtrlDtoBackspace - Enabled" : L"CtrlDtoBackspace - Disabled");
}

bool TrayAdd(HWND wnd, bool enabled) {
  Fill(wnd, enabled);
  g_added = Shell_NotifyIconW(NIM_ADD, &g_nid) != FALSE;
  return g_added;
}

void TrayUpdate(bool enabled) {
  if(!g_added) {
    return;
  }
  Fill(g_nid.hWnd, enabled);
  Shell_NotifyIconW(NIM_MODIFY, &g_nid);
}

void TrayRemove() {
  if(!g_added) {
    return;
  }
  Shell_NotifyIconW(NIM_DELETE, &g_nid);
  g_added = false;
}

void TrayShowMenu(HWND wnd) {
  bool on = AppIsEnabled();
  HMENU menu = CreatePopupMenu();
  if(!menu) {
    return;
  }

  AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, on ? L"Status: Enabled" : L"Status: Disabled");
  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(menu, MF_STRING | (on ? MF_CHECKED : 0), IDM_ENABLE, L"Enable CtrlDtoBackspace");
  AppendMenuW(menu, MF_STRING | (on ? 0 : MF_CHECKED), IDM_DISABLE, L"Disable CtrlDtoBackspace");
  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(menu, MF_STRING, IDM_SETTINGS, L"Open Settings");
  AppendMenuW(menu, MF_STRING | (StartupIsEnabled() ? MF_CHECKED : 0), IDM_STARTUP, L"Start with Windows");
  AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
  AppendMenuW(menu, MF_STRING, IDM_EXIT, L"Exit");

  // need this, or menu not close when click outside
  SetForegroundWindow(wnd);
  POINT pt;
  GetCursorPos(&pt);
  TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, wnd, nullptr);
  PostMessageW(wnd, WM_NULL, 0, 0);
  DestroyMenu(menu);
}
