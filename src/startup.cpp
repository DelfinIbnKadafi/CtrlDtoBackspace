#include "startup.h"

static const wchar_t kRunKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t kValue[] = L"CtrlDtoBackspace";

bool StartupIsEnabled() {
  HKEY key;
  if(RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
    return false;
  }
  LONG r = RegQueryValueExW(key, kValue, nullptr, nullptr, nullptr, nullptr);
  RegCloseKey(key);
  return r == ERROR_SUCCESS;
}

bool StartupSet(bool enable) {
  HKEY key;
  if(RegCreateKeyExW(HKEY_CURRENT_USER, kRunKey, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr) != ERROR_SUCCESS) {
    return false;
  }

  LONG r;
  if(enable) {
    wchar_t path[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if(len == 0 || len >= MAX_PATH) {
      RegCloseKey(key);
      return false;
    }
    // same value name, so never duplicate
    wchar_t cmd[MAX_PATH + 32];
    wsprintfW(cmd, L"\"%s\" --minimized", path);
    r = RegSetValueExW(key, kValue, 0, REG_SZ, (const BYTE*)cmd, (lstrlenW(cmd) + 1) * sizeof(wchar_t));
  } else {
    r = RegDeleteValueW(key, kValue);
    if(r == ERROR_FILE_NOT_FOUND) {
      r = ERROR_SUCCESS;
    }
  }

  RegCloseKey(key);
  return r == ERROR_SUCCESS;
}
