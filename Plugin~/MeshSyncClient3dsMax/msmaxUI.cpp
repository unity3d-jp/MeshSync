#include "pch.h"
#include "msmaxContext.h"
#include "msmaxUtils.h"
#include "resource.h"

#define msmaxTitle L"UnityMeshSync"

#define msmaxMenuTitle_Window           L"Window"
#define msmaxMenuTitle_Cache            L"Export Cache"
#define msmaxMenuTitle_ExportScene      L"Send Scene"
#define msmaxMenuTitle_ExportAnimations L"Send Animations"
#define msmaxMenuTitle_Import           L"Import"
#define msmaxActionID_Window            1
#define msmaxActionID_Cache             2
#define msmaxActionID_ExportScene       3
#define msmaxActionID_ExportAnimations  4
#define msmaxActionID_Import            5

static const ActionTableId      kTableActions = 0xec29063a;
static const ActionContextId    kTableContext = 0xec29063b;
static const MenuContextId      kMenuContext  = 0xec29063c;

class msmaxAction_Window : public ActionItem
{
public:
    int GetId() override { return msmaxActionID_Window; }
    void GetButtonText(MSTR& buttonText) override   { buttonText = MSTR(msmaxMenuTitle_Window); }
    void GetMenuText(MSTR& menuText) override       { menuText = MSTR(msmaxMenuTitle_Window); }
    void GetDescriptionText(MSTR& descText) override{ descText = MSTR(msmaxMenuTitle_Window); }
    void GetCategoryText(MSTR& catText) override    { catText = MSTR(msmaxTitle); }
    BOOL IsChecked() override       { return msmaxGetContext().isSettingWindowOpened(); }
    BOOL IsItemVisible() override   { return TRUE; }
    BOOL IsEnabled() override       { return TRUE; }
    void DeleteThis() override      { delete this; }

    BOOL ExecuteAction() override
    {
        if(msmaxGetContext().isSettingWindowOpened())
            msmaxGetContext().closeSettingWindow();
        else
            msmaxGetContext().openSettingWindow();
        return TRUE;
    }
};

class msmaxAction_Cache : public ActionItem
{
public:
    int GetId() override { return msmaxActionID_Cache; }
    void GetButtonText(MSTR& buttonText) override { buttonText = MSTR(msmaxMenuTitle_Cache); }
    void GetMenuText(MSTR& menuText) override { menuText = MSTR(msmaxMenuTitle_Cache); }
    void GetDescriptionText(MSTR& descText) override { descText = MSTR(msmaxMenuTitle_Cache); }
    void GetCategoryText(MSTR& catText) override { catText = MSTR(msmaxTitle); }
    BOOL IsChecked() override { return msmaxGetContext().isCacheWindowOpened(); }
    BOOL IsItemVisible() override { return TRUE; }
    BOOL IsEnabled() override { return TRUE; }
    void DeleteThis() override { delete this; }

    BOOL ExecuteAction() override
    {
        if (msmaxGetContext().isCacheWindowOpened())
            msmaxGetContext().closeCacheWindow();
        else
            msmaxGetContext().openCacheWindow();
        return TRUE;
    }
};

class msmaxAction_SendScene : public ActionItem
{
public:
    int GetId() override { return msmaxActionID_ExportScene; }
    void GetButtonText(MSTR& buttonText) override   { buttonText = MSTR(msmaxMenuTitle_ExportScene); }
    void GetMenuText(MSTR& menuText) override       { menuText = MSTR(msmaxMenuTitle_ExportScene); }
    void GetDescriptionText(MSTR& descText) override{ descText = MSTR(msmaxMenuTitle_ExportScene); }
    void GetCategoryText(MSTR& catText) override    { catText = MSTR(msmaxTitle); }
    BOOL IsChecked() override       { return FALSE; }
    BOOL IsItemVisible() override   { return TRUE; }
    BOOL IsEnabled() override       { return TRUE; }
    void DeleteThis() override      { delete this; }

    BOOL ExecuteAction() override
    {
        return msmaxSendScene(msmaxExportTarget::Objects, msmaxObjectScope::All);
    }
};

class msmaxAction_SendAnimations : public ActionItem
{
public:
    int GetId() override { return msmaxActionID_ExportAnimations; }
    void GetButtonText(MSTR& buttonText) override   { buttonText = MSTR(msmaxMenuTitle_ExportAnimations); }
    void GetMenuText(MSTR& menuText) override       { menuText = MSTR(msmaxMenuTitle_ExportAnimations); }
    void GetDescriptionText(MSTR& descText) override{ descText = MSTR(msmaxMenuTitle_ExportAnimations); }
    void GetCategoryText(MSTR& catText) override    { catText = MSTR(msmaxTitle); }
    BOOL IsChecked() override       { return FALSE; }
    BOOL IsItemVisible() override   { return TRUE; }
    BOOL IsEnabled() override       { return TRUE; }
    void DeleteThis() override      { delete this; }

    BOOL ExecuteAction() override
    {
        return msmaxSendScene(msmaxExportTarget::Animations, msmaxObjectScope::All);
    }
};

class msmaxAction_Import : public ActionItem
{
public:
    int GetId() override { return msmaxActionID_Import; }
    void GetButtonText(MSTR& buttonText) override   { buttonText = MSTR(msmaxMenuTitle_Import); }
    void GetMenuText(MSTR& menuText) override       { menuText = MSTR(msmaxMenuTitle_Import); }
    void GetDescriptionText(MSTR& descText) override{ descText = MSTR(msmaxMenuTitle_Import); }
    void GetCategoryText(MSTR& catText) override    { catText = MSTR(msmaxTitle); }
    BOOL IsChecked() override       { return FALSE; }
    BOOL IsItemVisible() override   { return TRUE; }
    BOOL IsEnabled() override       { return TRUE; }
    void DeleteThis() override      { delete this; }

    BOOL ExecuteAction() override
    {
        msmaxGetContext().recvScene();
        return TRUE;
    }
};

class msmaxActionCallback : public ActionCallback
{
public:
};
static msmaxActionCallback g_msmaxActionCallback;

void msmaxContext::registerMenu()
{
    unregisterMenu();

    auto *action_manager = GetCOREInterface()->GetActionManager();
    auto *menu_manager = GetCOREInterface()->GetMenuManager();
    {
        auto *table = new ActionTable(kTableActions, kTableContext, TSTR(msmaxTitle));
        table->AppendOperation(new msmaxAction_Window());
        table->AppendOperation(new msmaxAction_Cache());
        table->AppendOperation(new msmaxAction_SendScene());
        table->AppendOperation(new msmaxAction_SendAnimations());
        table->AppendOperation(new msmaxAction_Import());
        action_manager->RegisterActionTable(table);
        action_manager->ActivateActionTable(&g_msmaxActionCallback, kTableActions);
        action_manager->RegisterActionContext(kTableContext, msmaxTitle);
    }

    auto *table = action_manager->FindTable(kTableActions);
    auto *main_menu = menu_manager->GetMainMenuBar();

    bool first_time = menu_manager->RegisterMenuBarContext(kMenuContext, msmaxTitle);
    auto* menu = menu_manager->FindMenu(msmaxTitle);
    if (menu) {
        while (menu->NumItems() > 0)
            menu->RemoveItem(0);
    }
    else {
        menu = GetIMenu();
        menu->SetTitle(msmaxTitle);
        menu_manager->RegisterMenu(menu);
    }

    if (first_time) {
        // add menu to main menu bar if first time
        auto item = GetIMenuItem();
        item->SetActionItem(table->GetAction(msmaxActionID_Window));
        item->SetSubMenu(menu);
        main_menu->AddItem(item);
    }

    {
        auto item = GetIMenuItem();
        item->SetActionItem(table->GetAction(msmaxActionID_Window));
        menu->AddItem(item);
    }
    {
        auto item = GetIMenuItem();
        item->SetActionItem(table->GetAction(msmaxActionID_Cache));
        menu->AddItem(item);
    }
    {
        auto item = GetIMenuItem();
        item->SetActionItem(table->GetAction(msmaxActionID_ExportScene));
        menu->AddItem(item);
    }
    {
        auto item = GetIMenuItem();
        item->SetActionItem(table->GetAction(msmaxActionID_ExportAnimations));
        menu->AddItem(item);
    }
    //{
    //    auto item = GetIMenuItem();
    //    item->SetActionItem(table->GetAction(msmaxActionID_Import));
    //    menu->AddItem(item);
    //}
}


void msmaxContext::unregisterMenu()
{
    // nothing to do for now
}


extern HINSTANCE g_msmax_hinstance;
HWND g_msmax_settings_window;
HWND g_msmax_cache_window;

HWND g_msmax_current_window;

static bool CtrlIsChecked(int cid)
{
    return IsDlgButtonChecked(g_msmax_current_window, cid);
};
static void CtrlSetCheck(int cid, bool v)
{
    CheckDlgButton(g_msmax_current_window, cid, v);
};

static int CtrlGetInt(int cid, int default_value, bool sign = true)
{
    BOOL valid;
    int v = GetDlgItemInt(g_msmax_current_window, cid, &valid, sign);
    return valid ? v : default_value;
};

static float CtrlGetFloat(int cid, float default_value)
{
    BOOL valid;
    float v = GetDlgItemFloat(g_msmax_current_window, cid, &valid);
    return valid ? v : default_value;
};


static std::string CtrlGetText(int cid)
{
    wchar_t buf[256];
    GetDlgItemText(g_msmax_current_window, IDC_SERVER, buf, _countof(buf));
    return mu::ToMBS(buf);
}
static void CtrlSetText(int cid, const std::string& v)
{
    SetDlgItemTextA(g_msmax_current_window, cid, v.c_str());
}
static void CtrlSetText(int cid, int v)
{
    wchar_t buf[256];
    swprintf(buf, L"%d", v);
    SetDlgItemText(g_msmax_current_window, cid, buf);

}
static void CtrlSetText(int cid, float v)
{
    wchar_t buf[256];
    swprintf(buf, L"%.3f", v);
    SetDlgItemText(g_msmax_current_window, cid, buf);
}
static void CtrlComboAddString(int cid, const std::string& v)
{
    SendDlgItemMessageA(g_msmax_current_window, cid, CB_ADDSTRING, 0, (LPARAM)v.c_str());
}
static void CtrlComboSetSelection(int cid, int index)
{
    SendDlgItemMessageA(g_msmax_current_window, cid, CB_SETCURSEL, (WPARAM)index, 0);
}
static int CtrlComboGetSelection(int cid)
{
    return (int)SendDlgItemMessageA(g_msmax_current_window, cid, CB_GETCURSEL, 0, 0);
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

    auto& ctx = msmaxGetContext();
    auto& s = msmaxGetSettings();
    g_msmax_current_window = hDlg;

    INT_PTR ret = FALSE;
    switch (msg) {
    case WM_INITDIALOG:
        ret = TRUE;
        g_msmax_settings_window = hDlg;
        GetCOREInterface()->RegisterDlgWnd(g_msmax_settings_window);
        PositionWindowNearCursor(hDlg);
        ctx.updateSettingControls();
        ShowWindow(hDlg, SW_SHOW);
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

        auto handle_edit = [&](const auto& body) {
            switch (code) {
            case EN_SETFOCUS:
                DisableAccelerators();
                ret = TRUE;
                break;
            case EN_KILLFOCUS:
                body();
                EnableAccelerators();
                ret = TRUE;
                break;
            }
        };

        auto handle_button = [&](const auto& body) {
            switch (code) {
            case BN_CLICKED:
                body();
                ret = TRUE;
                break;
            }
        };

        auto notify_scene_update = []() {
            auto& ctx = msmaxGetContext();
            ctx.addDeferredCall([&ctx]() {
                ctx.onSceneUpdated();
                ctx.update();
            });
        };

        switch (cid) {
        case IDC_SERVER:
            handle_edit([&]() { s.client_settings.server = CtrlGetText(IDC_SERVER); });
            break;
        case IDC_PORT:
            handle_edit([&]() {
                int tmp = CtrlGetInt(IDC_PORT, s.client_settings.port);
                if (tmp < 0 || tmp > 63335) {
                    tmp = mu::clamp((int)tmp, 0, 63335);
                    CtrlSetText(IDC_PORT, tmp);
                }
                s.client_settings.port = tmp;
            });
            break;
        case IDC_SCALE_FACTOR:
            handle_edit([&]() {
                s.scale_factor = CtrlGetFloat(IDC_SCALE_FACTOR, s.scale_factor);
                notify_scene_update();
            });
            break;
        case IDC_SYNC_MESHES:
            handle_button([&]() {
                s.sync_meshes = CtrlIsChecked(IDC_SYNC_MESHES);
                notify_scene_update();
            });
            break;
        case IDC_MAKE_DOUBLE_SIDED:
            handle_button([&]() {
                s.make_double_sided = CtrlIsChecked(IDC_MAKE_DOUBLE_SIDED);
                notify_scene_update();
            });
            break;
        case IDC_IGNORE_NON_RENDERABLE:
            handle_button([&]() {
                s.ignore_non_renderable = CtrlIsChecked(IDC_IGNORE_NON_RENDERABLE);
                notify_scene_update();
            });
            break;
        case IDC_BAKE_MODIFIERS:
            handle_button([&]() {
                s.bake_modifiers = CtrlIsChecked(IDC_BAKE_MODIFIERS);
                if (!s.bake_modifiers && CtrlIsChecked(IDC_USE_RENDER_MESHES)) {
                    s.use_render_meshes = false;
                    CtrlSetCheck(IDC_USE_RENDER_MESHES, false);
                }
                notify_scene_update();
            });
            break;
        case IDC_USE_RENDER_MESHES:
            handle_button([&]() {
                if (CtrlIsChecked(IDC_USE_RENDER_MESHES)) {
                    if (!s.bake_modifiers) {
                        s.bake_modifiers = true;
                        CtrlSetCheck(IDC_BAKE_MODIFIERS, true);
                    }
                    s.use_render_meshes = true;
                }
                else {
                    s.use_render_meshes = false;
                }
                notify_scene_update();
            });
            break;
        case IDC_SYNC_BLENDSHAPES:
            handle_button([&]() {
                s.sync_blendshapes = CtrlIsChecked(IDC_SYNC_BLENDSHAPES);
                notify_scene_update();
            });
            break;
        case IDC_SYNC_BONES:
            handle_button([&]() {
                s.sync_bones = CtrlIsChecked(IDC_SYNC_BONES);
                notify_scene_update();
            });
            break;
        case IDC_SYNC_TEXTURES:
            handle_button([&]() {
                s.sync_textures = CtrlIsChecked(IDC_SYNC_TEXTURES);
                notify_scene_update();
            });
            break;
        case IDC_SYNC_CAMERAS:
            handle_button([&]() {
                s.sync_cameras = CtrlIsChecked(IDC_SYNC_CAMERAS);
                notify_scene_update();
            });
            break;
        case IDC_SYNC_LIGHTS:
            handle_button([&]() {
                s.sync_lights = CtrlIsChecked(IDC_SYNC_LIGHTS);
                notify_scene_update();
            });
            break;
        case IDC_AUTO_SYNC:
            handle_button([&]() {
                auto& ctx = msmaxGetContext();
                auto& settings = msmaxGetSettings();
                if (CtrlIsChecked(IDC_AUTO_SYNC)) {
                    if (ctx.isServerAvailable()) {
                        settings.auto_sync = true;
                        notify_scene_update();
                    }
                    else {
                        ctx.logInfo("MeshSync: Server not available. %s", ctx.getErrorMessage().c_str());
                        CtrlSetCheck(IDC_AUTO_SYNC, false);
                    }
                }
                else {
                    settings.auto_sync = false;
                }
            });
            break;

        case IDC_ANIMATION_TIME_SCALE:
            handle_edit([&]() {
                float tmp = CtrlGetFloat(IDC_ANIMATION_TIME_SCALE, s.anim_time_scale);
                if (tmp < 0.01f) {
                    tmp = mu::max(tmp, 0.01f);
                    CtrlSetText(IDC_ANIMATION_TIME_SCALE, tmp);
                }
                s.anim_time_scale = tmp;
            });
            break;
        case IDC_ANIMATION_SAMPLE_RATE:
            handle_edit([&]() {
                float tmp = CtrlGetFloat(IDC_ANIMATION_SAMPLE_RATE, s.anim_sample_rate);
                if (tmp < 0.01f) {
                    tmp = mu::max(tmp, 0.01f);
                    CtrlSetText(IDC_ANIMATION_SAMPLE_RATE, tmp);
                }
                s.anim_sample_rate = tmp;
            });
            break;
        case IDC_KEYFRAME_REDUCTION:
            handle_button([&]() {
                s.anim_keyframe_reduction = CtrlIsChecked(IDC_KEYFRAME_REDUCTION);
            });
            break;
        case IDC_KEEP_FLAT_CURVES:
            handle_button([&]() {
                s.anim_keep_flat_curves = CtrlIsChecked(IDC_KEEP_FLAT_CURVES);
            });
            break;
        case IDC_MANUAL_SYNC:
            handle_button([&]() { msmaxSendScene(msmaxExportTarget::Objects, msmaxObjectScope::All); });
            break;
        case IDC_SYNC_ANIMATIONS:
            handle_button([&]() { msmaxSendScene(msmaxExportTarget::Animations, msmaxObjectScope::All); });
            break;

        case IDC_BUTTON_EXPORT_CACHE:
            handle_button([&]() { ctx.openCacheWindow(); });
            break;
        default: break;
        }
        break;
    }

    default:
        break;
    }
    return ret;
}

void msmaxContext::openSettingWindow()
{
    if (!g_msmax_settings_window) {
        CreateDialogParam(g_msmax_hinstance, MAKEINTRESOURCE(IDD_SETTINGS_WINDOW),
            GetCOREInterface()->GetMAXHWnd(), msmaxSettingWindowCB, (LPARAM)this);
    }
}

void msmaxContext::closeSettingWindow()
{
    if (g_msmax_settings_window) {
        PostMessage(g_msmax_settings_window, WM_CLOSE, 0, 0);
    }
}

bool msmaxContext::isSettingWindowOpened() const
{
    return g_msmax_settings_window != nullptr;
}

void msmaxContext::updateSettingControls()
{
    auto& s = m_settings;
    CtrlSetText(IDC_SERVER, s.client_settings.server);
    CtrlSetText(IDC_PORT, (int)s.client_settings.port);

    CtrlSetText(IDC_SCALE_FACTOR,           s.scale_factor);
    CtrlSetCheck(IDC_SYNC_MESHES,           s.sync_meshes);
    CtrlSetCheck(IDC_MAKE_DOUBLE_SIDED,     s.make_double_sided);
    CtrlSetCheck(IDC_IGNORE_NON_RENDERABLE, s.ignore_non_renderable);
    CtrlSetCheck(IDC_BAKE_MODIFIERS,        s.bake_modifiers);
    CtrlSetCheck(IDC_USE_RENDER_MESHES,     s.use_render_meshes);
    CtrlSetCheck(IDC_SYNC_BLENDSHAPES,      s.sync_blendshapes);
    CtrlSetCheck(IDC_SYNC_BONES,            s.sync_bones);
    CtrlSetCheck(IDC_SYNC_TEXTURES,         s.sync_textures);
    CtrlSetCheck(IDC_SYNC_CAMERAS,          s.sync_cameras);
    CtrlSetCheck(IDC_SYNC_LIGHTS,           s.sync_lights);
    CtrlSetCheck(IDC_AUTO_SYNC,             s.auto_sync);

    CtrlSetText(IDC_ANIMATION_TIME_SCALE,   s.anim_time_scale);
    CtrlSetText(IDC_ANIMATION_SAMPLE_RATE,  s.anim_sample_rate);
    CtrlSetCheck(IDC_KEYFRAME_REDUCTION,    s.anim_keyframe_reduction);

    CtrlSetText(IDC_TXT_VERSION, "Plugin Version: " msPluginVersionStr);
}



static INT_PTR CALLBACK msmaxCacheWindowCB(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto& ctx = msmaxGetContext();
    auto& s = msmaxGetCacheSettings();
    g_msmax_current_window = hDlg;

    INT_PTR ret = FALSE;
    switch (msg) {
    case WM_INITDIALOG:
        ret = TRUE;
        g_msmax_cache_window = hDlg;

        // init combo boxes
        CtrlComboAddString(IDC_MATERIAL_RANGE, "None");
        CtrlComboAddString(IDC_MATERIAL_RANGE, "One Frame");
        CtrlComboAddString(IDC_MATERIAL_RANGE, "All Frames");

        GetCOREInterface()->RegisterDlgWnd(g_msmax_cache_window);
        PositionWindowNearCursor(hDlg);
        ctx.updateCacheControls();

        ShowWindow(hDlg, SW_SHOW);
        break;

    case WM_CLOSE:
        DestroyWindow(hDlg);
        break;

    case WM_DESTROY:
        GetCOREInterface()->UnRegisterDlgWnd(g_msmax_cache_window);
        g_msmax_cache_window = nullptr;
        break;

    case WM_COMMAND:
    {
        int code = HIWORD(wParam);
        int cid = LOWORD(wParam);

        auto handle_edit = [&](const auto& body) {
            switch (code) {
            case EN_SETFOCUS:
                DisableAccelerators();
                ret = TRUE;
                break;
            case EN_KILLFOCUS:
                body();
                EnableAccelerators();
                ret = TRUE;
                break;
            }
        };

        auto handle_button = [&](const auto& body) {
            switch (code) {
            case BN_CLICKED:
                body();
                ret = TRUE;
                break;
            }
        };

        auto handle_combo = [&](const auto& body) {
            switch (code) {
            case CBN_SELCHANGE:
                body();
                ret = TRUE;
                break;
            }
        };

        auto notify_scene_update = []() {
            auto& ctx = msmaxGetContext();
            ctx.addDeferredCall([&ctx]() {
                ctx.onSceneUpdated();
                ctx.update();
            });
        };

        switch (cid) {
        case IDC_OBJSCOPE_ALL:
            handle_button([&]() {
                s.object_scope = msmaxObjectScope::All;
            });
            break;
        case IDC_OBJSCOPE_SELECTED:
            handle_button([&]() {
                s.object_scope = msmaxObjectScope::Selected;
            });
            break;

        case IDC_FRAMERANGE_SINGLE:
            handle_button([&]() {
                s.frame_range = msmaxFrameRange::CurrentFrame;
            });
            break;
        case IDC_FRAMERANGE_ACTIVE:
            handle_button([&]() {
                s.frame_range = msmaxFrameRange::AllFrames;
            });
            break;
        case IDC_FRAMERANGE_CUSTOM:
            handle_button([&]() {
                s.frame_range = msmaxFrameRange::CustomRange;
            });
            break;

        case IDC_FRAME_BEGIN:
            handle_edit([&]() {
                int tmp = CtrlGetInt(IDC_FRAME_BEGIN, s.frame_begin);
                tmp = mu::clamp(tmp, 0, INT_MAX);
                CtrlSetText(IDC_FRAME_BEGIN, tmp);
                s.frame_begin = tmp;
            });
            break;
        case IDC_FRAME_END:
            handle_edit([&]() {
                int tmp = CtrlGetInt(IDC_FRAME_END, s.frame_end);
                tmp = mu::clamp(tmp, 0, INT_MAX);
                CtrlSetText(IDC_FRAME_END, tmp);
                s.frame_end = tmp;
            });
            break;

        case IDC_CACHE_SAMPLES_PER_FRAME:
            handle_edit([&]() {
                float tmp = CtrlGetFloat(IDC_CACHE_SAMPLES_PER_FRAME, s.sample_rate);
                tmp = mu::clamp(tmp, 0.01f, 100.0f);
                CtrlSetText(IDC_CACHE_SAMPLES_PER_FRAME, tmp);
                s.sample_rate = tmp;
            });
            break;
        case IDC_ZSTD_COMPRESSION_LEVEL:
            handle_edit([&]() {
                int tmp = CtrlGetInt(IDC_ZSTD_COMPRESSION_LEVEL, s.zstd_compression_level);
                tmp = mu::clamp(tmp, 0, 22);
                CtrlSetText(IDC_ZSTD_COMPRESSION_LEVEL, tmp);
                s.zstd_compression_level = tmp;
            });
            break;
        case IDC_MATERIAL_RANGE:
            handle_edit([&]() {
                s.material_frame_range = (msmaxMaterialFrameRange)CtrlComboGetSelection(IDC_MATERIAL_RANGE);
            });
            break;

            
        case IDC_BAKE_MODIFIERS:
            handle_button([&]() {
                s.bake_modifiers = CtrlIsChecked(IDC_BAKE_MODIFIERS);
            });
            break;
        case IDC_USE_RENDER_MESHES:
            handle_button([&]() {
                s.use_render_meshes = CtrlIsChecked(IDC_USE_RENDER_MESHES);
            });
            break;
        case IDC_FLATTEN_HIERARCHY:
            handle_button([&]() {
                s.flatten_hierarchy = CtrlIsChecked(IDC_FLATTEN_HIERARCHY);
            });
            break;

        case IDC_STRIP_NORMALS:
            handle_button([&]() {
                s.strip_normals = CtrlIsChecked(IDC_STRIP_NORMALS);
            });
            break;
        case IDC_STRIP_TANGENTS:
            handle_button([&]() {
                s.strip_tangents = CtrlIsChecked(IDC_STRIP_TANGENTS);
            });
            break;

        case IDOK:
            handle_button([&]() { msmaxExportCache(s); });
            DestroyWindow(hDlg);
            break;
        case IDCANCEL:
            DestroyWindow(hDlg);
            break;
        default: break;
        }
        break;
    }

    default:
        break;
    }
    return ret;
}

void msmaxContext::openCacheWindow()
{
    if (!g_msmax_cache_window) {
        // open file save dialog
        auto ifs = GetCOREInterface8();
        MSTR filename = mu::ToWCS(GetCurrentMaxFileName() + ".sc").c_str();
        MSTR dir = L"";

        int filter_index = 0;
        FilterList filter_list;
        filter_list.Append(_M("Scene cache files(*.sc)"));
        filter_list.Append(_M("*.sc"));
        filter_list.SetFilterIndex(filter_index);

        if (ifs->DoMaxSaveAsDialog(ifs->GetMAXHWnd(), L"Export Scene Cache", filename, dir, filter_list)) {
            msmaxGetCacheSettings().path = ms::ToMBS(filename);

            // open cache export settings window
            CreateDialogParam(g_msmax_hinstance, MAKEINTRESOURCE(IDD_CACHE_WINDOW),
                ifs->GetMAXHWnd(), msmaxCacheWindowCB, (LPARAM)this);
        }
    }
}

void msmaxContext::closeCacheWindow()
{
    if (g_msmax_cache_window) {
        PostMessage(g_msmax_cache_window, WM_CLOSE, 0, 0);
    }
}

bool msmaxContext::isCacheWindowOpened() const
{
    return g_msmax_cache_window != nullptr;
}

void msmaxContext::updateCacheControls()
{
    auto& s = m_cache_settings;

    switch (s.object_scope) {
    case msmaxObjectScope::All:
        CtrlSetCheck(IDC_OBJSCOPE_ALL, true);
        break;
    case msmaxObjectScope::Selected:
        CtrlSetCheck(IDC_OBJSCOPE_SELECTED, true);
        break;
    }

    switch (s.frame_range) {
    case msmaxFrameRange::CurrentFrame:
        CtrlSetCheck(IDC_FRAMERANGE_SINGLE, true);
        break;
    case msmaxFrameRange::AllFrames:
        CtrlSetCheck(IDC_FRAMERANGE_ACTIVE, true);
        break;
    case msmaxFrameRange::CustomRange:
        CtrlSetCheck(IDC_FRAMERANGE_CUSTOM, true);
        break;
    }

    CtrlSetText(IDC_FRAME_BEGIN, s.frame_begin);
    CtrlSetText(IDC_FRAME_END, s.frame_end);
    CtrlSetText(IDC_CACHE_SAMPLES_PER_FRAME, s.sample_rate);
    CtrlSetText(IDC_ZSTD_COMPRESSION_LEVEL, s.zstd_compression_level);

    CtrlComboSetSelection(IDC_MATERIAL_RANGE, (int)s.material_frame_range);
    CtrlSetCheck(IDC_BAKE_MODIFIERS, s.bake_modifiers);
    CtrlSetCheck(IDC_USE_RENDER_MESHES, s.use_render_meshes);
    CtrlSetCheck(IDC_FLATTEN_HIERARCHY, s.flatten_hierarchy);

    CtrlSetCheck(IDC_STRIP_NORMALS, s.strip_normals);
    CtrlSetCheck(IDC_STRIP_TANGENTS, s.strip_tangents);
}
