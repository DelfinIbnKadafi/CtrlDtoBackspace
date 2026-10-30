#ifndef STARTUP_H
#define STARTUP_H

#include <windows.h>

// run key has our value or not
bool StartupIsEnabled();

// add or remove run key (HKCU, no admin)
bool StartupSet(bool enable);

#endif
