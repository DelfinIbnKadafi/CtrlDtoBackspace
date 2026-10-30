#ifndef TRAY_H
#define TRAY_H

#include <windows.h>

// tray send this message to our window
#define WM_TRAY (WM_APP + 1)

bool TrayAdd(HWND wnd, bool enabled);
void TrayUpdate(bool enabled);
void TrayRemove();
void TrayShowMenu(HWND wnd);

#endif
