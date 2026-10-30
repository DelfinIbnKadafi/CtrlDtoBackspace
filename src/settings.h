#ifndef SETTINGS_H
#define SETTINGS_H

#include <windows.h>

// show settings window (or bring to front)
void SettingsShow(HWND owner);

// update controls, if window is open
void SettingsRefresh();

// call in message loop, true = message used by dialog
bool SettingsDialogMessage(MSG* msg);

#endif
