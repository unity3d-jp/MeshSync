#include "pch.h"
#include "MeshSyncClient3dsMax.h"
#include "msmaxUtils.h"
#include "resource.h"

#define msmaxTitle L"MeshSync"

#define msmaxMenuTitle_GUI              L"MeshSync GUI"
#define msmaxMenuTitle_ExportScene      L"MeshSync Export Scene"
#define msmaxMenuTitle_ExportAnimations L"MeshSync Export Animations"
#define msmaxActionID_GUI               0
#define msmaxActionID_ExportScene       1
#define msmaxActionID_ExportAnimations  2

static const ActionTableId kTableActions = 0xec29063a;
static const ActionContextId kTableContext = 0xec29063a;

class msmaxAction_GUI : public ActionItem
{
public:
    int GetId() override { return msmaxActionID_GUI; }
    void GetButtonText(MSTR& buttonText) override   { buttonText.FromBSTR(msmaxMenuTitle_GUI); }
    void GetMenuText(MSTR& menuText) override       { menuText.FromBSTR(msmaxMenuTitle_GUI); }
    void GetDescriptionText(MSTR& descText) override{ descText.FromBSTR(msmaxMenuTitle_GUI); }
    void GetCategoryText(MSTR& catText) override    { catText.FromBSTR(msmaxMenuTitle_GUI); }
    BOOL IsChecked() override       { return FALSE; }
    BOOL IsItemVisible() override   { return TRUE; }
    BOOL IsEnabled() override       { return TRUE; }
    void DeleteThis() override      { delete this; }

    BOOL ExecuteAction() override
    {
        MeshSyncClient3dsMax::getInstance().showSettingsWindow();
        return TRUE;
    }
};

class msmaxAction_ExportScene : public ActionItem
{
public:
    int GetId() override { return msmaxActionID_ExportScene; }
    void GetButtonText(MSTR& buttonText) override { buttonText.FromBSTR(msmaxMenuTitle_ExportScene); }
    void GetMenuText(MSTR& menuText) override { menuText.FromBSTR(msmaxMenuTitle_ExportScene); }
    void GetDescriptionText(MSTR& descText) override { descText.FromBSTR(msmaxMenuTitle_ExportScene); }
    void GetCategoryText(MSTR& catText) override { catText.FromBSTR(msmaxMenuTitle_ExportScene); }
    BOOL IsChecked() override { return FALSE; }
    BOOL IsItemVisible() override { return TRUE; }
    BOOL IsEnabled() override { return TRUE; }
    void DeleteThis() override { delete this; }

    BOOL ExecuteAction() override
    {
        MeshSyncClient3dsMax::getInstance().sendScene(MeshSyncClient3dsMax::SendScope::All);
        return TRUE;
    }
};

class msmaxAction_ExportAnimations : public ActionItem
{
public:
    int GetId() override { return msmaxActionID_ExportAnimations; }
    void GetButtonText(MSTR& buttonText) override { buttonText.FromBSTR(msmaxMenuTitle_ExportAnimations); }
    void GetMenuText(MSTR& menuText) override { menuText.FromBSTR(msmaxMenuTitle_ExportAnimations); }
    void GetDescriptionText(MSTR& descText) override { descText.FromBSTR(msmaxMenuTitle_ExportAnimations); }
    void GetCategoryText(MSTR& catText) override { catText.FromBSTR(msmaxMenuTitle_ExportAnimations); }
    BOOL IsChecked() override { return FALSE; }
    BOOL IsItemVisible() override { return TRUE; }
    BOOL IsEnabled() override { return TRUE; }
    void DeleteThis() override { delete this; }

    BOOL ExecuteAction() override
    {
        MeshSyncClient3dsMax::getInstance().sendAnimations(MeshSyncClient3dsMax::SendScope::All);
        return TRUE;
    }
};

class msmaxActionCallback : public ActionCallback
{
public:
};
static msmaxActionCallback g_msmaxActionCallback;

void MeshSyncClient3dsMax::registerMenu()
{
    unregisterMenu();

    {
        auto *table = new ActionTable(kTableActions, kTableContext, TSTR(msmaxTitle));
        table->AppendOperation(new msmaxAction_GUI());
        table->AppendOperation(new msmaxAction_ExportScene());
        table->AppendOperation(new msmaxAction_ExportAnimations());
        GetCOREInterface()->GetActionManager()->RegisterActionTable(table);
        GetCOREInterface()->GetActionManager()->ActivateActionTable(&g_msmaxActionCallback, kTableActions);
    }

    //auto *table = GetCOREInterface()->GetActionManager()->FindTable(kTableActions);
    //auto *mainmenu = GetCOREInterface()->GetMenuManager()->GetMainMenuBar();

    //auto item = GetIMenuItem();
    //item->SetEnabled(true);
    //item->SetVisible(true);
    //item->SetTitle(msmaxMenuTitle_GUI);
    //item->SetActionItem(table->GetAction(msmaxActionID_GUI));

    //{
    //    for (int i = 0; i < mainmenu->NumItems(); ++i) {
    //        mscTraceW(L"  menu item: %s\n", mainmenu->GetItem(i)->GetTitle().data());
    //        if (wcscmp(mainmenu->GetItem(i)->GetTitle().data(), L"&Tools") == 0) {
    //            mainmenu->GetItem(i)->GetSubMenu()->AddItem(item);
    //        }

    //        auto submenu = mainmenu->GetItem(i)->GetSubMenu();
    //        if (submenu) {
    //            for (int j = 0; j < submenu->NumItems(); ++j) {
    //                mscTraceW(L"    menu item: %s\n", submenu->GetItem(j)->GetTitle().data());
    //            }
    //        }
    //    }
    //}
}

void MeshSyncClient3dsMax::unregisterMenu()
{
    if (auto *menu = GetCOREInterface()->GetMenuManager()->FindMenu(msmaxTitle))
        GetCOREInterface()->GetMenuManager()->UnRegisterMenu(menu);
}


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

static INT_PTR CALLBACK msmaxSettingWindowCB(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // see this about DisableAccelerators(), EnableAccelerators() and GetCOREInterface()->RegisterDlgWnd()
    // https://help.autodesk.com/view/3DSMAX/2018/ENU/?guid=__developer_3ds_max_sdk_features_user_interface_action_system_keyboard_accelerators_and_dialog_html

    auto *_this = &MeshSyncClient3dsMax::getInstance();
    auto& s = _this->getSettings();

    INT_PTR ret = FALSE;
    switch (msg) {
    case WM_INITDIALOG:
        ret = TRUE;
        g_msmax_settings_window = hDlg;
        GetCOREInterface()->RegisterDlgWnd(g_msmax_settings_window);
        PositionWindowNearCursor(hDlg);
        _this->updateUIText();
        break;

    case WM_CLOSE:
        DestroyWindow(hDlg);
        break;

    case WM_DESTROY:
        GetCOREInterface()->UnRegisterDlgWnd(g_msmax_settings_window);
        g_msmax_settings_window = nullptr;
        break;

    case WM_COMMAND:
    {
        int code = HIWORD(wParam);
        int cid = LOWORD(wParam);

        auto handle_edit = [&](const std::function<void()>& body) {
            switch (code) {
            case EN_SETFOCUS:
                DisableAccelerators();
                ret = TRUE;
                break;
            case EN_KILLFOCUS:
                EnableAccelerators();
                ret = TRUE;
                break;
            case EN_CHANGE:
                ret = TRUE;
                body();
                break;
            }
        };

        auto handle_button = [&](const std::function<void()>& body) {
            switch (code) {
            case BN_CLICKED:
                body();
                ret = TRUE;
                break;
            }
        };

        if (cid == IDC_EDIT_SERVER)
            handle_edit([&]() { s.client_settings.server = CtrlGetText(IDC_EDIT_SERVER); });
        else if (cid == IDC_EDIT_PORT)
            handle_edit([&]() { s.client_settings.port = CtrlGetInt(IDC_EDIT_SERVER, s.client_settings.port); });
        else if (cid == IDC_EDIT_SCALE_FACTOR)
            handle_edit([&]() { s.scale_factor = CtrlGetFloat(IDC_EDIT_SCALE_FACTOR, s.scale_factor); });
        else if (cid == IDC_CHECK_MESHES)
            handle_button([&]() { s.sync_meshes = CtrlGetBool(IDC_CHECK_MESHES); });
        else if (cid == IDC_CHECK_NORMALS)
            handle_button([&]() { s.sync_normals = CtrlGetBool(IDC_CHECK_NORMALS); });
        else if (cid == IDC_CHECK_UVS)
            handle_button([&]() { s.sync_uvs = CtrlGetBool(IDC_CHECK_UVS); });
        else if (cid == IDC_CHECK_COLORS)
            handle_button([&]() { s.sync_colors = CtrlGetBool(IDC_CHECK_COLORS); });
        else if (cid == IDC_CHECK_BLENDSHAPES)
            handle_button([&]() { s.sync_blendshapes = CtrlGetBool(IDC_CHECK_BLENDSHAPES); });
        else if (cid == IDC_CHECK_BONES)
            handle_button([&]() { s.sync_bones = CtrlGetBool(IDC_CHECK_BONES); });
        else if (cid == IDC_CHECK_CAMERAS)
            handle_button([&]() { s.sync_cameras = CtrlGetBool(IDC_CHECK_CAMERAS); });
        else if (cid == IDC_CHECK_LIGHTS)
            handle_button([&]() { s.sync_lights = CtrlGetBool(IDC_CHECK_LIGHTS); });
        else if (cid == IDC_CHECK_AUTO_SYNC)
            handle_button([&]() { s.auto_sync = CtrlGetBool(IDC_CHECK_AUTO_SYNC); });
        else if (cid == IDC_EDIT_ANIMATION_TIME_SCALE)
            handle_edit([&]() { s.animation_time_scale = CtrlGetFloat(IDC_EDIT_ANIMATION_TIME_SCALE, s.animation_time_scale); });
        else if (cid == IDC_EDIT_ANIMATION_SPS)
            handle_edit([&]() { s.animation_sps = CtrlGetFloat(IDC_EDIT_ANIMATION_SPS, s.animation_sps); });
        else if (cid == IDC_BUTTON_MANUAL_SYNC)
            handle_button([&]() { _this->sendScene(MeshSyncClient3dsMax::SendScope::All); });
        else if (cid == IDC_BUTTON_SYNC_ANIMATIONS)
            handle_button([&]() { _this->sendAnimations(MeshSyncClient3dsMax::SendScope::All); });
        break;
    }

    default:
        break;
    }
    return ret;
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
