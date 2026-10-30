# CtrlDtoBackspace

Small native Windows tray app. Press **CTRL + D** and Windows get **Backspace**.
Good when your physical Backspace key is broken. Cause my backspace is broken.

- Windows 10 / 11, C++ with WinAPI only (no runtime, no framework)
- No admin needed to run, no service, no network, no logging
- Typed text is never saved or sent

## How it works

- `SetWindowsHookEx(WH_KEYBOARD_LL)` see every key. The hook runs on the main thread, so the normal message loop is enough (no polling, no extra thread, no timer).
- New **D** press + CTRL down + no Shift/Alt/Win -> the D event is eaten. We send: `CTRL up, Backspace down` with `SendInput`.
- Holding D: Windows repeat D by keyboard repeat rate. Each repeat is eaten and become one Backspace down (same speed as real Backspace).
- D release -> `Backspace up`, then CTRL down again if CTRL is still hold.
- Left and right CTRL both work.
- Our own events carry a mark in `dwExtraInfo`, hook skip them (no loop).
- Normal D (no CTRL) pass through. Other combos (Ctrl+Shift+D, AltGr+D, Win+D) are not touched.

## Tray menu

Enable / Disable (current state is checked), Open Settings, Start with Windows, Exit.
Disable remove the hook completely, so zero cost when off.

## Startup

`HKCU\Software\Microsoft\Windows\CurrentVersion\Run` value `CtrlDtoBackspace` = `"<exe>" --minimized`.
Same value name every time, so no duplicate. No admin needed.
Manual launch obey the "Launch minimized" setting. Autostart is always tray-only.

## Command line

- `--minimized` : no settings window
- `--quit` : close the running copy (installer use it)

## Build

Need CMake 3.15+ and MinGW-w64 or MSVC (x64).

MinGW:

    cmake -S . -B build -G "MinGW Makefiles"
    cmake --build build

MSVC (Developer Command Prompt):

    cmake -S . -B build -A x64
    cmake --build build --config MinSizeRel

Output: `build/CtrlDtoBackspace.exe` (static, small).

## Installer

1. Install Inno Setup 6.3 or newer.
2. Build the exe first.
3. Open `installer/setup.iss` in Inno Setup and press Compile (or `iscc installer\setup.iss`).
4. Result: `dist/CtrlDtoBackspace-Setup.exe`.

Wizard pages: Welcome, License (must accept), Information and Consent (must check the box), Install folder, Additional tasks, Progress, Finish.

## Known limits

- Windows (UIPI) does not let a normal app hook or send keys to **admin** windows (Task Manager as admin, elevated terminal). CTRL + D is normal there.
- Secure screens (UAC prompt, CTRL+ALT+DEL, lock screen) are not remapped.
- If Windows ever drop the hook, use tray: Disable then Enable.
- Key is matched by letter D (Dvorak etc: the key that type "D").

## License

MIT, see `LICENSE`.
