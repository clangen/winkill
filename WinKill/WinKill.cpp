#include "stdafx.h"
#include "WinKill.h"
#include "WinKillDialog.h"

BEGIN_MESSAGE_MAP(WinKillApp, CWinApp)
END_MESSAGE_MAP()

WinKillApp::WinKillApp() {
}

WinKillApp theApp;

BOOL WinKillApp::InitInstance()
{
    WinKillDialog dlg;
    dlg.DoModal();
    return FALSE;
}
