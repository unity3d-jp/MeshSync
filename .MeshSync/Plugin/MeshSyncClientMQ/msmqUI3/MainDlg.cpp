#include "pch.h"
#include "MainDlg.h"
#include "../MeshSyncClientMQ3.h"

CAppModule _Module;
CMessageLoop theLoop;


CMainDlg::CMainDlg(MeshSyncClientPlugin *plugin)
    : m_plugin(plugin)
{
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
    return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle()
{
    UIUpdateChildWindows();
    return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    // center the dialog on the screen
    CenterWindow();

    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    UIAddChildWindowContainer(m_hWnd);

    m_edit_server.Attach(GetDlgItem(IDC_EDIT_SERVER));
    m_edit_port.Attach(GetDlgItem(IDC_EDIT_PORT));
    m_edit_scale.Attach(GetDlgItem(IDC_EDIT_SCALEFACTOR));
    m_check_vcolor.Attach(GetDlgItem(IDC_CHECK_VCOLOR));
    m_check_bothsided.Attach(GetDlgItem(IDC_CHECK_BOTHSIDED));
    m_check_textures.Attach(GetDlgItem(IDC_CHECK_TEXTURES));
    m_check_camera.Attach(GetDlgItem(IDC_CHECK_CAMERA));
    m_edit_camera_path.Attach(GetDlgItem(IDC_EDIT_CAMERA_PATH));
    m_check_autosync.Attach(GetDlgItem(IDC_CHECK_AUTOSYNC));
    m_check_bake_skin.Attach(GetDlgItem(IDC_CHECK_BAKE_SKIN));
    m_check_bake_cloth.Attach(GetDlgItem(IDC_CHECK_BAKE_CLOTH));
    m_txt_version.Attach(GetDlgItem(IDC_TXT_VERSION));
    m_txt_log.Attach(GetDlgItem(IDC_TXT_LOG));

    auto& s = getSettings();
    char buf[256];
    m_edit_server.SetWindowText(s.client_settings.server.c_str());
    sprintf(buf, "%d", (int)s.client_settings.port);
    m_edit_port.SetWindowText(buf);
    sprintf(buf, "%.3f", s.scale_factor);
    m_edit_scale.SetWindowText(buf);
    m_check_vcolor.SetCheck(s.sync_vertex_color);
    m_check_textures.SetCheck(s.sync_textures);
    m_check_camera.SetCheck(s.sync_camera);
    m_edit_camera_path.SetWindowText(s.host_camera_path.c_str());
    m_check_autosync.SetCheck(s.auto_sync);
    m_check_bake_skin.SetCheck(s.bake_skin);
    m_check_bake_cloth.SetCheck(s.bake_cloth);
    m_txt_version.SetWindowTextA("Plugin Version: " msPluginVersionStr);
    m_initializing = false;
    return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    // unregister message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveMessageFilter(this);
    pLoop->RemoveIdleHandler(this);

    return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CloseDialog(wID);
    return 0;
}

void CMainDlg::CloseDialog(int nVal)
{
    ShowWindow(SW_HIDE);
    m_plugin->WindowClose();
}


LRESULT CMainDlg::OnEnChangeEditServer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
    char buf[1024];
    ::GetWindowTextA(hWndCtl, buf, sizeof(buf));
    getSettings().client_settings.server = buf;
    return 0;
}


LRESULT CMainDlg::OnEnChangeEditPort(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
    char buf[1024];
    ::GetWindowTextA(hWndCtl, buf, sizeof(buf));
    getSettings().client_settings.port = std::atoi(buf);
    return 0;
}

LRESULT CMainDlg::OnEnChangeScaleFactor(WORD, WORD, HWND hWndCtl, BOOL &)
{
    if (m_initializing)
        return 0;

    char buf[1024];
    ::GetWindowTextA(hWndCtl, buf, sizeof(buf));
    auto scale = std::atof(buf);
    if (scale != 0.0) {
        getSettings().scale_factor = (float)scale;
        m_plugin->AutoSyncMeshes();
    }
    return 0;
}

LRESULT CMainDlg::OnBnClickedCheckVcolor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_initializing)
        return 0;

    getSettings().sync_vertex_color = m_check_vcolor.GetCheck() != 0;
    m_plugin->AutoSyncMeshes();
    return 0;
}

LRESULT CMainDlg::OnBnClickedCheckBothSided(WORD, WORD, HWND, BOOL &)
{
    if (m_initializing)
        return 0;

    getSettings().make_double_sided = m_check_bothsided.GetCheck() != 0;
    m_plugin->AutoSyncMeshes();
    return 0;
}

LRESULT CMainDlg::OnBnClickedCheckTexture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_initializing)
        return 0;

    getSettings().sync_textures = m_check_textures.GetCheck() != 0;
    m_plugin->AutoSyncMeshes();
    return 0;
}

LRESULT CMainDlg::OnBnClickedCheckCamera(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_initializing)
        return 0;

    getSettings().sync_camera = m_check_camera.GetCheck() != 0;
    m_plugin->AutoSyncCamera();
    return 0;
}

LRESULT CMainDlg::OnEnChangeCameraPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
    if (m_initializing)
        return 0;

    char buf[1024];
    ::GetWindowTextA(hWndCtl, buf, sizeof(buf));
    getSettings().host_camera_path = buf;
    return 0;
}

LRESULT CMainDlg::OnBnClickedCheckAutosync(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
    if (m_initializing)
        return 0;

    if (m_check_autosync.GetCheck()) {
        auto& ctx = m_plugin->getContext();
        if (ctx.isServerAvailable()) {
            getSettings().auto_sync = true;
            m_plugin->Export();
        }
        else {
            m_check_autosync.SetCheck(0);
            LogInfo(ctx.getErrorMessage().c_str());
        }
    }
    else {
        getSettings().auto_sync = false;
    }
    return 0;
}

LRESULT CMainDlg::OnBnClickedButtonSync(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_initializing)
        return 0;

    m_plugin->Export();
    return 0;
}


LRESULT CMainDlg::OnBnClickedCheckBakeSkin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_initializing)
        return 0;

    getSettings().bake_skin = m_check_bake_skin.GetCheck() != 0;
    return 0;
}
LRESULT CMainDlg::OnBnClickedCheckBakeCloth(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_initializing)
        return 0;

    getSettings().bake_cloth = m_check_bake_cloth.GetCheck() != 0;
    return 0;
}

LRESULT CMainDlg::OnBnClickedButtonImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (m_initializing)
        return 0;

    m_plugin->Import();
    return 0;
}

void CMainDlg::LogInfo(const char *message)
{
    m_txt_log.SetWindowTextA(message);
}

msmqSettings& CMainDlg::getSettings()
{
    return m_plugin->getContext().getSettings();
}


//---------------------------------------------------------------------------
//  DllMain
//---------------------------------------------------------------------------
BOOL APIENTRY DllMain(HINSTANCE hInstance,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        HRESULT hRes = ::CoInitialize(NULL);
        // If you are running on NT 4.0 or higher you can use the following call instead to 
        // make the EXE free threaded. This means that calls come in on a random RPC thread.
        //	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
        ATLASSERT(SUCCEEDED(hRes));

        // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
        ::DefWindowProc(NULL, 0, 0, 0L);

        AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

        hRes = _Module.Init(NULL, hInstance);
        ATLASSERT(SUCCEEDED(hRes));

        _Module.AddMessageLoop(&theLoop);

        return SUCCEEDED(hRes);
    }

    if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        _Module.RemoveMessageLoop();

        _Module.Term();
        ::CoUninitialize();
    }

    return TRUE;
}


static std::unique_ptr<CMainDlg> g_dlg;

void InitializeSettingDialog(MeshSyncClientPlugin *plugin, HWND parent)
{
    g_dlg.reset(new CMainDlg(plugin));
    g_dlg->Create(parent);
    //g_dlg->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void FinalizeSettingDialog()
{
    if (g_dlg) {
        g_dlg->DestroyWindow();
        g_dlg.reset();
    }
}

void ShowSettingDialog(bool show)
{
    if (g_dlg) {
        g_dlg->ShowWindow(show ? SW_SHOW : SW_HIDE);
    }
}

bool IsSettingDialogActive()
{
    return g_dlg && g_dlg->IsWindowVisible();
}

void msmqLogInfo(const char *message)
{
    if (g_dlg)
        g_dlg->LogInfo(message);
}
