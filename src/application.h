#ifndef APPLICATION_H
#define APPLICATION_H

#include <windows.h>
#include "resource.h"

#define APP_NAME L"CtrlDtoBackspace"
#define APP_VERSION_W L"" APP_VERSION_STR
#define APP_WND_CLASS L"CtrlDtoBackspaceWnd"
#define APP_MUTEX_NAME L"CtrlDtoBackspace_SingleInstance"

// second instance send this, show settings
#define WM_SHOW_SETTINGS (WM_APP + 2)

// start app, forceMinimized = no settings window
bool AppInit(HINSTANCE inst, bool forceMinimized);

bool AppIsEnabled();
bool AppSetEnabled(bool on);

bool AppStartMinimized();
void AppSetStartMinimized(bool on);

// ask app to close
void AppExit();

#endif
