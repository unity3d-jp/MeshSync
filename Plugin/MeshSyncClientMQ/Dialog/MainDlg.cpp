#include "stdafx.h"
#include <string>
#include <memory>
#include "MainDlg.h"

CAppModule _Module;
CMessageLoop theLoop;

std::string& GetServer(MeshSyncClientPlugin *plugin);
uint16_t& GetPort(MeshSyncClientPlugin *plugin);
bool& GetAutoSync(MeshSyncClientPlugin *plugin);
void Send(MeshSyncClientPlugin *plugin);
void Import(MeshSyncClientPlugin *plugin);


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
    m_check_autosync.Attach(GetDlgItem(IDC_CHECK_AUTOSYNC));

    char buf[256];
    m_edit_server.SetWindowText(GetServer(m_plugin).c_str());
    sprintf(buf, "%d", (int)GetPort(m_plugin));
    m_edit_port.SetWindowText(buf);
    m_check_autosync.SetCheck(GetAutoSync(m_plugin));

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
}


LRESULT CMainDlg::OnEnChangeEditServer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
    char buf[1024];
    ::GetWindowTextA(hWndCtl, buf, sizeof(buf));
    GetServer(m_plugin) = buf;
    return 0;
}


LRESULT CMainDlg::OnEnChangeEditPort(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
    char buf[1024];
    ::GetWindowTextA(hWndCtl, buf, sizeof(buf));
    GetPort(m_plugin) = atoi(buf);
    return 0;
}


LRESULT CMainDlg::OnBnClickedCheckAutosync(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
    GetAutoSync(m_plugin) = m_check_autosync.GetCheck();
    return 0;
}


LRESULT CMainDlg::OnBnClickedButtonSync(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    Send(m_plugin);
    return 0;
}


LRESULT CMainDlg::OnBnClickedButtonImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    Import(m_plugin);
    return 0;
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
