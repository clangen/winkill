#include "stdafx.h"
#include "WinKill.h"
#include "WinKillDialog.h"

#include <WinKillHook.h>

#define SetAppIcon(x) \
    SetIcon(x, TRUE); \
    SetIcon(x, FALSE); \
    SetTrayIcon(x);

#define WM_MYTRAYICON WM_USER + 2000
#define MENU_ITEM_TOGGLE 1983
#define MENU_ITEM_EXIT 1979
#define MENU_ITEM_TOGGLE_CAPTION L"Toggle"
#define MENU_ITEM_EXIT_CAPTION L"Exit"
#define TRAY_ICON_TIP L"WinKill v0.2"

BEGIN_MESSAGE_MAP(WinKillDialog, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

WinKillDialog::WinKillDialog(CWnd* parent)
: CDialog(WinKillDialog::IDD, parent)
, mActiveIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME))
, mKilledIcon(AfxGetApp()->LoadIconW(IDI_KILLED))
, mHooked(false)
, mTrayIconVisible(false)
{
}

void WinKillDialog::ShowTrayIcon() {
    if (mTrayIconVisible) {
        return;
    }

    SecureZeroMemory(&mTrayIcon, sizeof(mTrayIcon));
    mTrayIcon.cbSize = sizeof(mTrayIcon);
    mTrayIcon.hWnd = m_hWnd;
    mTrayIcon.uID = 0;
    mTrayIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    mTrayIcon.uCallbackMessage = WM_MYTRAYICON;
    mTrayIcon.hIcon = mKilledIcon;
    ::wcscpy_s(mTrayIcon.szTip, 255, TRAY_ICON_TIP);

    mTrayIconVisible = (Shell_NotifyIcon(NIM_ADD, &mTrayIcon) != 0);

    if (mTrayIconVisible){
        mTrayIcon.uVersion = NOTIFYICON_VERSION;
        ::Shell_NotifyIcon(NIM_SETVERSION, &mTrayIcon);
    }
}

void WinKillDialog::HideTrayIcon() {
    if (!mTrayIconVisible) {
        return;
    }

    mTrayIconVisible = !(Shell_NotifyIcon(NIM_DELETE, &mTrayIcon) != 0);
}

void WinKillDialog::SetTrayIcon(HICON icon) {
    if (mTrayIconVisible) {
        mTrayIcon.hIcon = icon;
        Shell_NotifyIcon(NIM_MODIFY, &mTrayIcon);
    }
}

BOOL WinKillDialog::OnInitDialog() {
    CDialog::OnInitDialog();

    if (!mTrayMenu.CreatePopupMenu()) {
        return FALSE;
    }

    MENUITEMINFO menuItem = { 0 };
    menuItem.cbSize = sizeof(MENUITEMINFO);
    menuItem.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STRING;

    menuItem.dwTypeData = MENU_ITEM_TOGGLE_CAPTION;
    menuItem.cch = _tcslen(MENU_ITEM_TOGGLE_CAPTION) + 1;
    menuItem.wID = MENU_ITEM_TOGGLE;
    menuItem.fType = MFT_STRING;
    mTrayMenu.InsertMenuItem(0, &menuItem);

    menuItem.dwTypeData = L"-";
    menuItem.cch = 2;
    menuItem.fType = MFT_SEPARATOR;
    mTrayMenu.InsertMenuItem(0, &menuItem);

    menuItem.dwTypeData = MENU_ITEM_EXIT_CAPTION;
    menuItem.cch = _tcslen(MENU_ITEM_EXIT_CAPTION) + 1;
    menuItem.wID = MENU_ITEM_EXIT;
    menuItem.fType = MFT_STRING;
    mTrayMenu.InsertMenuItem(MENU_ITEM_EXIT, &menuItem);

    StartHook();
    ShowTrayIcon();

    return TRUE;
}

void WinKillDialog::OnPaint() {
    if (IsIconic()) {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, mKilledIcon);
    }
    else {
        CDialog::OnPaint();
    }
}

HCURSOR WinKillDialog::OnQueryDragIcon() {
    return (HCURSOR) mKilledIcon;
}

void WinKillDialog::StartHook() {
    mHooked = winkill_install_hook(this->GetSafeHwnd());

    if (mHooked) {
        // hook is in place. windows key is DISABLED!
        SetAppIcon(mKilledIcon);
    }
    else {
        MessageBox(L"Couldn't start keyboard hook!", L"WinKill", MB_OK);
    }
}

void WinKillDialog::StopHook() {
    mHooked = (!winkill_remove_hook());

    if (!mHooked) {
        // hook is no longer active. windows key is ENABLED!
        SetAppIcon(mActiveIcon);
    }
}

void WinKillDialog::ToggleHook() {
    mHooked ? StopHook() : StartHook();
}

void WinKillDialog::OnWindowPosChanging(WINDOWPOS* wndpos) {
    // always keep the main window hidden!
    if (wndpos->flags & SWP_SHOWWINDOW) {
        wndpos->flags &= ~SWP_SHOWWINDOW;
        PostMessage(WM_WINDOWPOSCHANGING, 0, (LPARAM)wndpos);
        ShowWindow(SW_HIDE);
    }
    else {
        CDialog::OnWindowPosChanging(wndpos);
    }
}

LRESULT WinKillDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch(message) {
    case WM_MYTRAYICON:
        switch (LOWORD(lParam)) {
        case WM_LBUTTONDOWN:
            ToggleHook();
            break;

        case WM_RBUTTONUP:
            {
                // SetForegroundWindow + PostMessage(WM_NULL) is a hack to prevent
                // "sticky popup menu syndrome." i hate you, win32api.
                POINT cursor = { 0 };
                ::GetCursorPos(&cursor);
                ::SetForegroundWindow(this->GetSafeHwnd());
                mTrayMenu.TrackPopupMenu(0, cursor.x, cursor.y, this);
                PostMessage(WM_NULL, 0, 0);
            }
            break;
        }
        return 0;

    case WM_DESTROY:
        {
            StopHook();
            HideTrayIcon();
        }
        break;

    case WM_COMMAND:
        {
            if (HIWORD(wParam) == 0) {
                switch (LOWORD(wParam)) {
                case MENU_ITEM_EXIT:
                    PostQuitMessage(0);
                    return 1;

                case MENU_ITEM_TOGGLE:
                    ToggleHook();
                    return 1;
                }
            }
        }
        break;
    }

    return CDialog::WindowProc(message, wParam, lParam);
}
