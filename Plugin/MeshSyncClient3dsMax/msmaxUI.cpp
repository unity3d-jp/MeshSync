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
    auto *_this = &MeshSyncClient3dsMax::getInstance();
    switch (msg) {
    case WM_INITDIALOG:
    {
        g_msmax_settings_window = hWnd;
        PositionWindowNearCursor(hWnd);
        _this->updateUIText();
        break;
    }
    case WM_CLOSE:
        g_msmax_settings_window = nullptr;
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        g_msmax_settings_window = nullptr;
        break;

    case WM_COMMAND:
    {
        int code = HIWORD(wParam);
        int cid = LOWORD(wParam);
        if (cid == IDC_BUTTON_MANUAL_SYNC && code == BN_CLICKED) {
            _this->sendScene(MeshSyncClient3dsMax::SendScope::All);
            break;
        }
        else if (cid == IDC_BUTTON_SYNC_ANIMATIONS && code == BN_CLICKED) {
            _this->sendAnimations(MeshSyncClient3dsMax::SendScope::All);
            break;
        }
    }

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

static wchar_t g_buf[256];

static inline const wchar_t* to_wstr(int v)
{
    swprintf(g_buf, L"%d", v);
    return g_buf;
}
static inline const wchar_t* to_wstr(float v)
{
    swprintf(g_buf, L"%.3f", v);
    return g_buf;
}

void MeshSyncClient3dsMax::updateUIText()
{
    auto& s = m_settings;
    SetDlgItemText(g_msmax_settings_window, IDC_EDIT_SERVER, mu::ToWCS(s.client_settings.server).c_str());
    SetDlgItemText(g_msmax_settings_window, IDC_EDIT_PORT, to_wstr((int)s.client_settings.port));

    SetDlgItemText(g_msmax_settings_window, IDC_EDIT_SCALE_FACTOR, to_wstr(s.scale_factor));
    CheckDlgButton(g_msmax_settings_window, IDC_CHECK_MESHES,       s.sync_meshes);
    CheckDlgButton(g_msmax_settings_window, IDC_CHECK_NORMALS,      s.sync_normals);
    CheckDlgButton(g_msmax_settings_window, IDC_CHECK_UVS,          s.sync_uvs);
    CheckDlgButton(g_msmax_settings_window, IDC_CHECK_COLORS,       s.sync_colors);
    CheckDlgButton(g_msmax_settings_window, IDC_CHECK_BLENDSHAPES,  s.sync_blendshapes);
    CheckDlgButton(g_msmax_settings_window, IDC_CHECK_BONES,        s.sync_bones);
    CheckDlgButton(g_msmax_settings_window, IDC_CHECK_CAMERAS,      s.sync_cameras);
    CheckDlgButton(g_msmax_settings_window, IDC_CHECK_LIGHTS,       s.sync_lights);

    SetDlgItemText(g_msmax_settings_window, IDC_EDIT_ANIMATION_TIME_SCALE, to_wstr(s.animation_time_scale));
    SetDlgItemText(g_msmax_settings_window, IDC_EDIT_ANIMATION_SPS, to_wstr(s.animation_sps));
}


void MeshSyncClient3dsMax::applyUISettings()
{
    auto& s = m_settings;
    BOOL valid;
    int iv;
    float fv;

    GetDlgItemText(g_msmax_settings_window, IDC_EDIT_SERVER, g_buf, _countof(g_buf));
    s.client_settings.server = mu::ToMBS(g_buf);

    iv = GetDlgItemInt(g_msmax_settings_window, IDC_EDIT_PORT, &valid, false);
    if (valid) s.client_settings.port = iv;

    fv = GetDlgItemFloat(g_msmax_settings_window, IDC_EDIT_SCALE_FACTOR, &valid);
    if (valid) s.scale_factor = fv;

    s.sync_meshes       = IsDlgButtonChecked(g_msmax_settings_window, IDC_CHECK_MESHES);
    s.sync_normals      = IsDlgButtonChecked(g_msmax_settings_window, IDC_CHECK_NORMALS);
    s.sync_uvs          = IsDlgButtonChecked(g_msmax_settings_window, IDC_CHECK_UVS);
    s.sync_colors       = IsDlgButtonChecked(g_msmax_settings_window, IDC_CHECK_COLORS);
    s.sync_blendshapes  = IsDlgButtonChecked(g_msmax_settings_window, IDC_CHECK_BLENDSHAPES);
    s.sync_bones        = IsDlgButtonChecked(g_msmax_settings_window, IDC_CHECK_BONES);
    s.sync_cameras      = IsDlgButtonChecked(g_msmax_settings_window, IDC_CHECK_CAMERAS);
    s.sync_lights       = IsDlgButtonChecked(g_msmax_settings_window, IDC_CHECK_LIGHTS);

    fv = GetDlgItemFloat(g_msmax_settings_window, IDC_EDIT_ANIMATION_TIME_SCALE, &valid);
    if (valid) s.animation_time_scale = fv;

    fv = GetDlgItemFloat(g_msmax_settings_window, IDC_EDIT_ANIMATION_SPS, &valid);
    if (valid) s.animation_sps = fv;
}

