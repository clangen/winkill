#include <StdAfx.h>
#include <Windows.h>
#include <Shellapi.h>
#include <tchar.h>
#include <WinKillHook.h>
#include "resource.h"

#define WM_MYTRAYICON WM_USER + 2000
#define MENU_ITEM_TOGGLE 1983
#define MENU_ITEM_EXIT 1979
#define MENU_ITEM_TOGGLE_CAPTION L"Toggle"
#define MENU_ITEM_EXIT_CAPTION L"Exit"
#define TRAY_ICON_TIP L"WinKill v0.3"
#define WINDOW_CLASS L"WinKillClass"

static HICON iconActive = nullptr, iconKilled = nullptr;
static bool hooked = false, trayIconDataVisible = false;
static HMENU trayMenu = 0;
static NOTIFYICONDATA trayIconData = { };
static HWND mainWindow;
static HINSTANCE instance;

static void showTrayIcon();
static void setTrayIcon(HICON icon);
static void hideTrayIcon();
static void createTrayMenu();
static void startHook();
static void stopHook();
static void toggleHook();
static void createWindow(HINSTANCE instance);
static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int CALLBACK wWinMain(HINSTANCE instance, HINSTANCE prev, LPWSTR args, int showType) {
    createWindow(instance);

    MSG msg = { };
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_MYTRAYICON: {
            switch (LOWORD(lParam)) {
                case WM_LBUTTONDOWN: {
                    toggleHook();
                    break;
                }

                case WM_RBUTTONUP: {
                        POINT cursor = { 0 };
                        ::GetCursorPos(&cursor);
                        ::SetForegroundWindow(mainWindow);
                        TrackPopupMenuEx(trayMenu, 0, cursor.x, cursor.y, hwnd, nullptr);
                        PostMessage(mainWindow, WM_NULL, 0, 0);
                    }
                    break;
            }
            return 0;
        }

        case WM_DESTROY: {
            stopHook();
            hideTrayIcon();
            break;
        }

        case WM_COMMAND: {
            if (HIWORD(wParam) == 0) {
                switch (LOWORD(wParam)) {
                case MENU_ITEM_EXIT:
                    PostQuitMessage(0);
                    return 1;

                case MENU_ITEM_TOGGLE:
                    toggleHook();
                    return 1;
                }
            }
            break;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void createWindow(HINSTANCE instance) {
    iconActive = LoadIcon(instance, MAKEINTRESOURCE(IDR_MAINFRAME));
    iconKilled = LoadIcon(instance, MAKEINTRESOURCE(IDI_KILLED));

    WNDCLASS wc = {};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"WinKillClass";
    RegisterClass(&wc);

    mainWindow =
        CreateWindowEx(
            WS_EX_TOOLWINDOW,
            WINDOW_CLASS,
            TRAY_ICON_TIP,
            0,
            0, 0, 0, 0, /* dimens */
            nullptr,
            nullptr,
            instance,
            nullptr);

    SetWindowLong(mainWindow, GWL_STYLE, 0); /* removes title, borders. */

    SetWindowPos(
        mainWindow,
        nullptr,
        -32000, -32000, 50, 50,
        SWP_FRAMECHANGED | SWP_SHOWWINDOW);

    createTrayMenu();
    showTrayIcon();
}

static void showTrayIcon() {
    if (trayIconDataVisible) {
        return;
    }

    SecureZeroMemory(&trayIconData, sizeof(trayIconData));
    trayIconData.cbSize = sizeof(trayIconData);
    trayIconData.hWnd = mainWindow;
    trayIconData.uID = 0;
    trayIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    trayIconData.uCallbackMessage = WM_MYTRAYICON;
    trayIconData.hIcon = hooked ? iconKilled : iconActive;
    ::wcscpy_s(trayIconData.szTip, 255, TRAY_ICON_TIP);

    trayIconDataVisible = (Shell_NotifyIcon(NIM_ADD, &trayIconData) != 0);

    if (trayIconDataVisible){
        trayIconData.uVersion = NOTIFYICON_VERSION;
        Shell_NotifyIcon(NIM_SETVERSION, &trayIconData);
    }
}

static void hideTrayIcon() {
    if (!trayIconDataVisible) {
        return;
    }

    trayIconDataVisible = !(Shell_NotifyIcon(NIM_DELETE, &trayIconData) != 0);
}

static void setTrayIcon(HICON icon) {
    if (trayIconDataVisible) {
        trayIconData.hIcon = icon;
        Shell_NotifyIcon(NIM_MODIFY, &trayIconData);
    }
}

static void createTrayMenu() {
    trayMenu = CreatePopupMenu();

    MENUITEMINFO menuItem = { 0 };
    menuItem.cbSize = sizeof(MENUITEMINFO);
    menuItem.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING;

    AppendMenu(trayMenu, 0, MENU_ITEM_TOGGLE, MENU_ITEM_TOGGLE_CAPTION);
    AppendMenu(trayMenu, MF_SEPARATOR, MENU_ITEM_TOGGLE, L"-");
    AppendMenu(trayMenu, 0, MENU_ITEM_EXIT, MENU_ITEM_EXIT_CAPTION);

    //MyCommandLineInfo commandLine;
    //AfxGetApp()->ParseCommandLine(commandLine);
    //commandLine.startDisabled ? StopHook() : StartHook();

    showTrayIcon();
}

static void startHook() {
    hooked = winkill_install_hook(mainWindow);

    if (hooked) {
        setTrayIcon(iconKilled);
    }
    else {
        MessageBox(mainWindow, L"Couldn't start keyboard hook!", L"WinKill", MB_OK);
    }
}

static void stopHook() {
    hooked = (!winkill_remove_hook());

    if (!hooked) {
        setTrayIcon(iconActive);
    }
}

void toggleHook() {
    hooked ? stopHook() : startHook();
}