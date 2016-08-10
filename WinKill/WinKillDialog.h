#pragma once

class WinKillDialog : public CDialog {
public:
    WinKillDialog(CWnd* parent = NULL);
    enum { IDD = IDD_WINKILL_DIALOG };

protected:
    virtual BOOL OnInitDialog();
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual void OnWindowPosChanging(WINDOWPOS* wndpos);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();

    DECLARE_MESSAGE_MAP()

private:
    void StartHook();
    void StopHook();
    void ToggleHook();
    void ShowTrayIcon();
    void SetTrayIcon(HICON icon);
    void HideTrayIcon();

private:
    HICON mActiveIcon, mKilledIcon;
    bool mHooked, mTrayIconVisible;
    CMenu mTrayMenu;
    NOTIFYICONDATA mTrayIcon;
};
