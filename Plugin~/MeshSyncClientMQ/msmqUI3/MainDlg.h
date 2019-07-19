#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlframe.h>
#include <atlctrls.h>

extern CAppModule _Module;

#include <atlwin.h>
#include "resource.h"

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

class MeshSyncClientPlugin;
struct msmqSettings;

class CMainDlg : public CDialogImpl<CMainDlg>, public CUpdateUI<CMainDlg>,
        public CMessageFilter, public CIdleHandler
{
public:
    enum { IDD = IDD_MAINDLG };

    CMainDlg(MeshSyncClientPlugin *plugin);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnIdle();

    BEGIN_UPDATE_UI_MAP(CMainDlg)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CMainDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_HANDLER(IDC_EDIT_SERVER, EN_CHANGE, OnEnChangeEditServer)
        COMMAND_HANDLER(IDC_EDIT_PORT, EN_CHANGE, OnEnChangeEditPort)
        COMMAND_HANDLER(IDC_EDIT_SCALEFACTOR, EN_CHANGE, OnEnChangeScaleFactor)
        COMMAND_HANDLER(IDC_CHECK_VCOLOR, BN_CLICKED, OnBnClickedCheckVcolor)
        COMMAND_HANDLER(IDC_CHECK_BOTHSIDED, BN_CLICKED, OnBnClickedCheckBothSided)
        COMMAND_HANDLER(IDC_CHECK_TEXTURES, BN_CLICKED, OnBnClickedCheckTexture)
        COMMAND_HANDLER(IDC_CHECK_CAMERA, BN_CLICKED, OnBnClickedCheckCamera)
        COMMAND_HANDLER(IDC_EDIT_CAMERA_PATH, EN_CHANGE, OnEnChangeCameraPath)
        COMMAND_HANDLER(IDC_CHECK_AUTOSYNC, BN_CLICKED, OnBnClickedCheckAutosync)
        COMMAND_HANDLER(IDC_BUTTON_SYNC, BN_CLICKED, OnBnClickedButtonSync)
        COMMAND_HANDLER(IDC_CHECK_BAKE_SKIN, BN_CLICKED, OnBnClickedCheckBakeSkin)
        COMMAND_HANDLER(IDC_CHECK_BAKE_CLOTH, BN_CLICKED, OnBnClickedCheckBakeCloth)
        COMMAND_HANDLER(IDC_BUTTON_IMPORT, BN_CLICKED, OnBnClickedButtonImport)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    void CloseDialog(int nVal);

    LRESULT OnEnChangeEditServer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEnChangeEditPort(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEnChangeScaleFactor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckVcolor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckBothSided(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckTexture(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckCamera(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEnChangeCameraPath(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckAutosync(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedButtonSync(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckBakeSkin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckBakeCloth(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedButtonImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    void LogInfo(const char *message);

private:
    msmqSettings& getSettings();

    MeshSyncClientPlugin *m_plugin = nullptr;
    CEdit m_edit_server;
    CEdit m_edit_port;
    CEdit m_edit_scale;
    CButton m_check_vcolor;
    CButton m_check_bothsided;
    CButton m_check_textures;
    CButton m_check_camera;
    CEdit m_edit_camera_path;
    CButton m_check_autosync;
    CButton m_check_bake_skin;
    CButton m_check_bake_cloth;
    CStatic m_txt_version;
    CStatic m_txt_log;
    bool m_initializing = true;
};
