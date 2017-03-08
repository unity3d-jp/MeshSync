// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class MeshSyncClientPlugin;

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
        COMMAND_HANDLER(IDC_CHECK_AUTOSYNC, BN_CLICKED, OnBnClickedCheckAutosync)
        COMMAND_HANDLER(IDC_BUTTON_SYNC, BN_CLICKED, OnBnClickedButtonSync)
        COMMAND_HANDLER(IDC_CHECK_BAKE_SKIN, BN_CLICKED, OnBnClickedCheckBakeSkin)
        COMMAND_HANDLER(IDC_CHECK_BAKE_CLOTH, BN_CLICKED, OnBnClickedCheckBakeCloth)
        COMMAND_HANDLER(IDC_BUTTON_IMPORT, BN_CLICKED, OnBnClickedButtonImport)
        COMMAND_HANDLER(IDC_CHECK_CAMERA, BN_CLICKED, OnBnClickedCheckCamera)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    void CloseDialog(int nVal);

    LRESULT OnEnChangeEditServer(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEnChangeEditPort(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnEnChangeScaleFactor(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckCamera(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckAutosync(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedButtonSync(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckBakeSkin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedCheckBakeCloth(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedButtonImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);


private:
    MeshSyncClientPlugin *m_plugin = nullptr;
    CEdit m_edit_server;
    CEdit m_edit_port;
    CEdit m_edit_scale;
    CButton m_check_camera;
    CButton m_check_autosync;
    CButton m_check_bake_skin;
    CButton m_check_bake_cloth;
};
