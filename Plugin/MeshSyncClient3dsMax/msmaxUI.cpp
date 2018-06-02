#include "pch.h"
#include "MeshSyncClient3dsMax.h"
#include "msmaxUtils.h"
#include "resource.h"


extern HINSTANCE g_msmax_hinstance;
HWND g_msmax_settings_window;

static bool CtrlGetBool(int cid)
{
    return IsDlgButtonChecked(g_msmax_settings_window, cid);
};
static void CtrlSetBool(int cid, bool v)
{
    CheckDlgButton(g_msmax_settings_window, cid, v);
};

static int CtrlGetInt(int cid, int default_value, bool sign = false)
{
    BOOL valid;
    int v = GetDlgItemInt(g_msmax_settings_window, cid, &valid, sign);
    return valid ? v : default_value;
};

static float CtrlGetFloat(int cid, float default_value)
{
    BOOL valid;
    float v = GetDlgItemFloat(g_msmax_settings_window, cid, &valid);
    return valid ? v : default_value;
};


static std::string CtrlGetText(int cid)
{
    wchar_t buf[256];
    GetDlgItemText(g_msmax_settings_window, IDC_EDIT_SERVER, buf, _countof(buf));
    return mu::ToMBS(buf);
}
static void CtrlSetText(int cid, const std::string& v)
{
    SetDlgItemText(g_msmax_settings_window, cid, mu::ToWCS(v).c_str());
}
static void CtrlSetText(int cid, int v)
{
    wchar_t buf[256];
    swprintf(buf, L"%d", v);
    SetDlgItemText(g_msmax_settings_window, cid, buf);

}
static void CtrlSetText(int cid, float v)
{
    wchar_t buf[256];
    swprintf(buf, L"%.3f", v);
    SetDlgItemText(g_msmax_settings_window, cid, buf);
}

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
    auto& s = _this->getSettings();

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
        if (cid == IDC_BUTTON_MANUAL_SYNC && code == BN_CLICKED)
            _this->sendScene(MeshSyncClient3dsMax::SendScope::All);
        else if (cid == IDC_BUTTON_SYNC_ANIMATIONS && code == BN_CLICKED)
            _this->sendAnimations(MeshSyncClient3dsMax::SendScope::All);
        else if (cid == IDC_CHECK_MESHES && code == BN_CLICKED)
            s.sync_meshes = CtrlGetBool(IDC_CHECK_MESHES);
        else if (cid == IDC_CHECK_NORMALS && code == BN_CLICKED)
            s.sync_normals = CtrlGetBool(IDC_CHECK_NORMALS);
        else if (cid == IDC_CHECK_UVS && code == BN_CLICKED)
            s.sync_uvs = CtrlGetBool(IDC_CHECK_UVS);
        else if (cid == IDC_CHECK_COLORS && code == BN_CLICKED)
            s.sync_colors = CtrlGetBool(IDC_CHECK_COLORS);
        else if (cid == IDC_CHECK_BLENDSHAPES && code == BN_CLICKED)
            s.sync_blendshapes = CtrlGetBool(IDC_CHECK_BLENDSHAPES);
        else if (cid == IDC_CHECK_BONES && code == BN_CLICKED)
            s.sync_bones = CtrlGetBool(IDC_CHECK_BONES);
        else if (cid == IDC_CHECK_CAMERAS && code == BN_CLICKED)
            s.sync_cameras = CtrlGetBool(IDC_CHECK_CAMERAS);
        else if (cid == IDC_CHECK_LIGHTS && code == BN_CLICKED)
            s.sync_lights = CtrlGetBool(IDC_CHECK_LIGHTS);
        else if (cid == IDC_CHECK_AUTO_SYNC && code == BN_CLICKED)
            s.auto_sync = CtrlGetBool(IDC_CHECK_AUTO_SYNC);
        break;
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

void MeshSyncClient3dsMax::updateUIText()
{
    auto& s = m_settings;
    CtrlSetText(IDC_EDIT_SERVER, s.client_settings.server);
    CtrlSetText(IDC_EDIT_PORT, (int)s.client_settings.port);

    CtrlSetText(IDC_EDIT_SCALE_FACTOR,  s.scale_factor);
    CtrlSetBool(IDC_CHECK_MESHES,       s.sync_meshes);
    CtrlSetBool(IDC_CHECK_NORMALS,      s.sync_normals);
    CtrlSetBool(IDC_CHECK_UVS,          s.sync_uvs);
    CtrlSetBool(IDC_CHECK_COLORS,       s.sync_colors);
    CtrlSetBool(IDC_CHECK_BLENDSHAPES,  s.sync_blendshapes);
    CtrlSetBool(IDC_CHECK_BONES,        s.sync_bones);
    CtrlSetBool(IDC_CHECK_CAMERAS,      s.sync_cameras);
    CtrlSetBool(IDC_CHECK_LIGHTS,       s.sync_lights);
    CtrlSetBool(IDC_CHECK_AUTO_SYNC,    s.auto_sync);

    CtrlSetText(IDC_EDIT_ANIMATION_TIME_SCALE, s.animation_time_scale);
    CtrlSetText(IDC_EDIT_ANIMATION_SPS,        s.animation_sps);
}


void MeshSyncClient3dsMax::applyUISettings()
{
    auto& s = m_settings;

    s.client_settings.server= CtrlGetText(IDC_EDIT_SERVER);
    s.client_settings.port  = CtrlGetInt(IDC_EDIT_PORT, s.client_settings.port);

    s.scale_factor          = CtrlGetFloat(IDC_EDIT_SCALE_FACTOR, s.scale_factor);
    s.sync_meshes           = CtrlGetBool(IDC_CHECK_MESHES);
    s.sync_normals          = CtrlGetBool(IDC_CHECK_NORMALS);
    s.sync_uvs              = CtrlGetBool(IDC_CHECK_UVS);
    s.sync_colors           = CtrlGetBool(IDC_CHECK_COLORS);
    s.sync_blendshapes      = CtrlGetBool(IDC_CHECK_BLENDSHAPES);
    s.sync_bones            = CtrlGetBool(IDC_CHECK_BONES);
    s.sync_cameras          = CtrlGetBool(IDC_CHECK_CAMERAS);
    s.sync_lights           = CtrlGetBool(IDC_CHECK_LIGHTS);
    s.auto_sync             = CtrlGetBool(IDC_CHECK_AUTO_SYNC);

    s.animation_time_scale  = CtrlGetFloat(IDC_EDIT_ANIMATION_TIME_SCALE, s.animation_time_scale);
    s.animation_sps         = CtrlGetFloat(IDC_EDIT_ANIMATION_SPS, s.animation_sps);
}

