#include "pch.h"
#include "MeshSyncClient3dsMax.h"
#include "msmaxUtils.h"
#include "resource.h"


extern HINSTANCE g_msmax_hinstance;
HWND g_msmax_settings_window;

static void PositionWindowNearCursor(HWND hwnd)
{
    RECT rect;
    POINT cur;
    GetWindowRect(hwnd, &rect);
    GetCursorPos(&cur);
    int scrwid = GetScreenWidth();
    int scrhgt = GetScreenHeight();
    int winwid = rect.right - rect.left;
    int winhgt = rect.bottom - rect.top;
    cur.x -= winwid / 3; // 1/3 of the window should be to the left of the cursor
    cur.y -= winhgt / 3; // 1/3 of the window should be above the cursor
    if (cur.x + winwid > scrwid - 10) cur.x = scrwid - winwid - 10; // check too far right 
    if (cur.x < 10) cur.x = 10; // check too far left
    if (cur.y + winhgt > scrhgt - 50) cur.y = scrhgt - 50 - winhgt; // check if too low
    if (cur.y < 10) cur.y = 10; // check for too high s45
    MoveWindow(hwnd, cur.x, cur.y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
}

static INT_PTR CALLBACK msmaxSettingWindowCB(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
    {
        PositionWindowNearCursor(hWnd);
        g_msmax_settings_window = hWnd;
        break;
    }
    case WM_CLOSE:
        g_msmax_settings_window = nullptr;
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        g_msmax_settings_window = nullptr;
        break;

    default:
        return 0;
    }
    return 1;
}

void MeshSyncClient3dsMax::showSettingsWindow()
{
    if (!g_msmax_settings_window) {
        CreateDialogParam(g_msmax_hinstance, MAKEINTRESOURCE(IDD_SETTINGS_WINDOW),
            GetCOREInterface()->GetMAXHWnd(), msmaxSettingWindowCB, (LPARAM)this);
    }
}

void MeshSyncClient3dsMax::closeSettingsWindow()
{
    if (g_msmax_settings_window) {
        PostMessage(g_msmax_settings_window, WM_CLOSE, 0, 0);
    }
}
