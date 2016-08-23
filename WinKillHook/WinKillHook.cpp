#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x401

#include <windows.h>
#include <winuser.h>

#include "WinKillHook.h"

HHOOK hook = NULL;
HWND hwnd = NULL;
HINSTANCE instance = NULL;

BOOL APIENTRY DllMain(HANDLE module, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        ::instance = (HINSTANCE) module;
        break;

    case DLL_PROCESS_DETACH:
        winkill_remove_hook();
        break;
    }

    return TRUE;
}

WINKILLHOOK_API LRESULT CALLBACK keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        switch (wParam) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP: {
                DWORD keyCode = ((PKBDLLHOOKSTRUCT) lParam)->vkCode;

                if ((keyCode == VK_LWIN) || (keyCode == VK_RWIN)) {
                    return 1;
                }
            }
            break;
        }
    }

    return ::CallNextHookEx(NULL, nCode, wParam, lParam);
}

WINKILLHOOK_API bool winkill_install_hook(HWND owner) {
    if ((!::hook) && owner) {
        ::hwnd = owner;
        ::hook = ::SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_proc, instance, NULL);
    }

    return (::hook != NULL);
}

WINKILLHOOK_API bool winkill_remove_hook() {
    if ((::hook) && (::UnhookWindowsHookEx(::hook))) {
        ::hook = NULL;
        ::hwnd = NULL;
    }

    return (::hook == NULL);
}
