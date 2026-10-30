#include "settings.h"
#include "application.h"
#include "startup.h"
#include "keyboard_hook.h"

static HWND g_dlg = nullptr;

static void SetCheck(int id, bool on) {
  CheckDlgButton(g_dlg, id, on ? BST_CHECKED : BST_UNCHECKED);
}

static bool GetCheck(int id) {
  return IsDlgButtonChecked(g_dlg, id) == BST_CHECKED;
}

void SettingsRefresh() {
  if(!g_dlg) {
    return;
  }
  SetCheck(IDC_CHK_ENABLE, AppIsEnabled());
  SetCheck(IDC_CHK_STARTUP, StartupIsEnabled());
  SetCheck(IDC_CHK_MINIMIZED, AppStartMinimized());

  const wchar_t* text = L"Status: Disabled (keyboard is normal)";
  if(AppIsEnabled() && KbHookActive()) {
    text = L"Status: Active (CTRL + D = Backspace)";
  }
  SetDlgItemTextW(g_dlg, IDC_STATUS, text);
  SetDlgItemTextW(g_dlg, IDC_VERSION, L"Version " APP_VERSION_W);
}

static INT_PTR CALLBACK DlgProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp) {
  switch(msg) {
    case WM_INITDIALOG:
      g_dlg = dlg;
      SettingsRefresh();
      return TRUE;

    case WM_COMMAND:
      if(HIWORD(wp) != BN_CLICKED) {
        return FALSE;
      }
      switch(LOWORD(wp)) {
        case IDC_CHK_ENABLE:
          AppSetEnabled(GetCheck(IDC_CHK_ENABLE));
          break;
        case IDC_CHK_STARTUP:
          if(!StartupSet(GetCheck(IDC_CHK_STARTUP))) {
            MessageBoxW(dlg, L"Cannot change startup setting.", APP_NAME, MB_OK | MB_ICONERROR);
          }
          break;
        case IDC_CHK_MINIMIZED:
          AppSetStartMinimized(GetCheck(IDC_CHK_MINIMIZED));
          break;
        case IDC_BTN_EXIT:
          AppExit();
          return TRUE;
        case IDOK:
        case IDCANCEL:
          DestroyWindow(dlg);
          return TRUE;
      }
      SettingsRefresh();
      return TRUE;

    case WM_CLOSE:
      DestroyWindow(dlg);
      return TRUE;

    case WM_DESTROY:
      g_dlg = nullptr;
      return TRUE;
  }
  return FALSE;
}

void SettingsShow(HWND owner) {
  if(!g_dlg) {
    CreateDialogParamW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDD_SETTINGS), owner, DlgProc, 0);
  }
  if(!g_dlg) {
    return;
  }
  SettingsRefresh();
  ShowWindow(g_dlg, SW_SHOW);
  SetForegroundWindow(g_dlg);
}

bool SettingsDialogMessage(MSG* msg) {
  return g_dlg != nullptr && IsDialogMessageW(g_dlg, msg) != FALSE;
}
