#include "keyboard_hook.h"

// mark for our own events, so we skip them
static const ULONG_PTR kMark = 0x43444254;

static HHOOK g_hook = nullptr;
static bool g_ctrlL = false;     // left ctrl is hold (only when remap run)
static bool g_ctrlR = false;     // right ctrl is hold (only when remap run)
static bool g_dPassed = false;   // normal D is down, we let it pass
static bool g_bsActive = false;  // ctrl+d remap is running

// make one key event
static INPUT MakeKey(WORD vk, bool up, bool ext) {
  INPUT in = {};
  in.type = INPUT_KEYBOARD;
  in.ki.wVk = vk;
  in.ki.wScan = (WORD)MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
  in.ki.dwFlags = (up ? KEYEVENTF_KEYUP : 0) | (ext ? KEYEVENTF_EXTENDEDKEY : 0);
  in.ki.dwExtraInfo = kMark;
  return in;
}

static bool Send(INPUT* in, UINT n) {
  return SendInput(n, in, sizeof(INPUT)) == n;
}

static bool IsDown(int vk) {
  return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

// shift, alt, win = not our combo (alt gr too)
static bool OtherModDown() {
  return IsDown(VK_SHIFT) || IsDown(VK_MENU) || IsDown(VK_LWIN) || IsDown(VK_RWIN);
}

// first D down: ctrl up, backspace down
static bool StartRemap() {
  INPUT in[3];
  UINT n = 0;
  if(g_ctrlL) {
    in[n++] = MakeKey(VK_LCONTROL, true, false);
  }
  if(g_ctrlR) {
    in[n++] = MakeKey(VK_RCONTROL, true, true);
  }
  in[n++] = MakeKey(VK_BACK, false, false);
  return Send(in, n);
}

// D hold repeat: backspace down again
static void RepeatRemap() {
  INPUT in = MakeKey(VK_BACK, false, false);
  Send(&in, 1);
}

// D up: backspace up, give ctrl back if still hold
static void StopRemap() {
  INPUT in[3];
  UINT n = 0;
  in[n++] = MakeKey(VK_BACK, true, false);
  if(g_ctrlL) {
    in[n++] = MakeKey(VK_LCONTROL, false, false);
  }
  if(g_ctrlR) {
    in[n++] = MakeKey(VK_RCONTROL, false, true);
  }
  Send(in, n);
  g_bsActive = false;
}

static LRESULT CALLBACK HookProc(int code, WPARAM wp, LPARAM lp) {
  if(code != HC_ACTION) {
    return CallNextHookEx(g_hook, code, wp, lp);
  }

  const KBDLLHOOKSTRUCT* k = (const KBDLLHOOKSTRUCT*)lp;

  // our own event, skip (no loop)
  if(k->dwExtraInfo == kMark) {
    return CallNextHookEx(g_hook, code, wp, lp);
  }

  bool down = (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN);
  bool up = (wp == WM_KEYUP || wp == WM_SYSKEYUP);

  // D up lost? (example: lock screen) then stop remap
  if(g_bsActive && k->vkCode != 'D' && !IsDown('D')) {
    StopRemap();
  }

  // ctrl key while remap run: eat it, only save state
  if(k->vkCode == VK_LCONTROL || k->vkCode == VK_RCONTROL) {
    if(g_bsActive) {
      bool isLeft = (k->vkCode == VK_LCONTROL);
      if(down) {
        if(isLeft) {
          g_ctrlL = true;
        } else {
          g_ctrlR = true;
        }
      } else if(up) {
        if(isLeft) {
          g_ctrlL = false;
        } else {
          g_ctrlR = false;
        }
      }
      return 1;
    }
    return CallNextHookEx(g_hook, code, wp, lp);
  }

  if(k->vkCode == 'D') {
    if(down) {
      // key repeat, send backspace again
      if(g_bsActive) {
        RepeatRemap();
        return 1;
      }
      // normal D still hold, not new press
      if(g_dPassed) {
        return CallNextHookEx(g_hook, code, wp, lp);
      }
      // new D press, check ctrl only (read real state)
      if(!OtherModDown()) {
        bool l = IsDown(VK_LCONTROL);
        bool r = IsDown(VK_RCONTROL);
        if(l || r) {
          g_ctrlL = l;
          g_ctrlR = r;
          if(StartRemap()) {
            g_bsActive = true;
            return 1;
          }
        }
      }
      // not ctrl+d, normal D
      g_dPassed = true;
      return CallNextHookEx(g_hook, code, wp, lp);
    }
    if(up) {
      if(g_bsActive) {
        StopRemap();
        return 1;
      }
      g_dPassed = false;
      return CallNextHookEx(g_hook, code, wp, lp);
    }
  }

  return CallNextHookEx(g_hook, code, wp, lp);
}

bool KbHookInstall() {
  if(g_hook) {
    return true;
  }
  g_bsActive = false;
  g_dPassed = false;
  g_hook = SetWindowsHookExW(WH_KEYBOARD_LL, HookProc, GetModuleHandleW(nullptr), 0);
  return g_hook != nullptr;
}

void KbHookRemove() {
  if(!g_hook) {
    return;
  }
  UnhookWindowsHookEx(g_hook);
  g_hook = nullptr;
  // if remap run, finish it clean
  if(g_bsActive) {
    StopRemap();
  }
}

bool KbHookActive() {
  return g_hook != nullptr;
}
