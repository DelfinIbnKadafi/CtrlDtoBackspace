#ifndef KEYBOARD_HOOK_H
#define KEYBOARD_HOOK_H

#include <windows.h>

// install global hook, true if ok
bool KbHookInstall();

// remove hook, keyboard back to normal
void KbHookRemove();

// hook is on or not
bool KbHookActive();

#endif
