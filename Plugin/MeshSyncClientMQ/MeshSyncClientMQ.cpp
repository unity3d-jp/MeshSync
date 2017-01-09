#include "pch.h"
#include "MeshSyncClientMQ.h"
#include "Dialog.h"

static MeshSyncClientPlugin g_plugin;

// Constructor
// コンストラクタ
MeshSyncClientPlugin::MeshSyncClientPlugin()
{
    dlgMain = NULL;
    m_bActivate = false;
    m_bOnDraw = false;
    m_bOnUpdateScene = false;
    m_bOnUpdateUndo = false;
    m_bWidgetEvents = false;
    m_bMouseMove = false;
}

// Destructor
// デストラクタ
MeshSyncClientPlugin::~MeshSyncClientPlugin()
{
    if (dlgMain != NULL) {
        // Also deletes the window here just to make sure though it has been 
        // deleted in Exit().
        // Exit()で破棄されるはずだが一応念のためここでも
        delete dlgMain;
        dlgMain = NULL;
    }
}

//---------------------------------------------------------------------------
//  GetPlugInID
//    プラグインIDを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::GetPlugInID(DWORD *Product, DWORD *ID)
{
    // プロダクト名(制作者名)とIDを、全部で64bitの値として返す
    // 値は他と重複しないようなランダムなもので良い
    *Product = 0x483ADF11;
    *ID = 0xB0CC9999;
}

//---------------------------------------------------------------------------
//  GetPlugInName
//    プラグイン名を返す。
//    この関数は[プラグインについて]表示時に呼び出される。
//---------------------------------------------------------------------------
const char *MeshSyncClientPlugin::GetPlugInName(void)
{
    return "Unity Mesh Sync       Copyright(C) 2017, i-saint.";
}

//---------------------------------------------------------------------------
//  EnumString
//    ポップアップメニューに表示される文字列を返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
const char *MeshSyncClientPlugin::EnumString(void)
{
    return "Unity Mesh Sync";
}


//---------------------------------------------------------------------------
//  EnumSubCommand
//    サブコマンド前を列挙
//---------------------------------------------------------------------------
const char *MeshSyncClientPlugin::EnumSubCommand(int index)
{
    switch (index) {
    case 0: return "Message";
    case 1: return "Vertex";
    case 2: return "Face";
    }
    return NULL;
}

//---------------------------------------------------------------------------
//  GetSubCommandString
//    サブコマンドの文字列を列挙
//---------------------------------------------------------------------------
const wchar_t *MeshSyncClientPlugin::GetSubCommandString(int index)
{
    switch (index) {
    case 0: return L"Message";
    case 1: return L"Vertex";
    case 2: return L"Face";
    }
    return NULL;
}

//---------------------------------------------------------------------------
//  Initialize
//    アプリケーションの初期化
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::Initialize()
{
    if (dlgMain == NULL) {
        // Create a new window that's parent is the main window.
        // メインウインドウを親にした新しいウィンドウの作成
        MQWindowBase mainwnd = MQWindow::GetMainWindow();
        dlgMain = new SettingsDlg(this, mainwnd);

        dlgMain->AddMessage(L"Initialize");
    }
    return TRUE;
}

//---------------------------------------------------------------------------
//  Exit
//    アプリケーションの終了
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::Exit()
{
    if (dlgMain != NULL) {
        dlgMain->AddMessage(L"Exit");
        delete dlgMain;
        dlgMain = NULL;
    }
}

//---------------------------------------------------------------------------
//  Activate
//    表示・非表示切り替え要求
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::Activate(MQDocument doc, BOOL flag)
{
    if (dlgMain == NULL)
        return FALSE;

    // Switch showing and hiding the window.
    // ウインドウの表示・非表示切り替え
    m_bActivate = flag ? true : false;
    dlgMain->SetVisible(m_bActivate);

    if (m_bActivate) {
        dlgMain->UpdateList(doc, false);
    }

    wchar_t buf[256];
    swprintf_s(buf, L"Activate : flag=%s", flag ? L"TRUE" : L"FALSE");
    dlgMain->AddMessage(buf);

    return m_bActivate;
}

//---------------------------------------------------------------------------
//  IsActivated
//    表示・非表示状態の返答
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::IsActivated(MQDocument doc)
{
    // Return TRUE if a window has been not created yet.
    // ウインドウがまだ生成されていないならTRUE
    if (dlgMain == NULL)
        return FALSE;

    return m_bActivate;
}

//---------------------------------------------------------------------------
//  OnMinimize
//    ウインドウの最小化への返答
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnMinimize(MQDocument doc, BOOL flag)
{
    wchar_t buf[256];
    swprintf_s(buf, L"OnMinimize : flag=%s", flag ? L"TRUE" : L"FALSE");
    dlgMain->AddMessage(buf);
}

//---------------------------------------------------------------------------
//  OnReceiveUserMessage
//    プラグイン独自のメッセージの受け取り
//---------------------------------------------------------------------------
int MeshSyncClientPlugin::OnReceiveUserMessage(MQDocument doc, DWORD src_product, DWORD src_id, const char *description, void *message)
{
    size_t len = strlen(description);
    wchar_t *buf = new wchar_t[len + 200];
    swprintf_s(buf, len + 200, L"OnReceiveUserMessage : src_plugin=[%08x %08x] description=%S", src_product, src_id, description);
    dlgMain->AddMessage(buf);

    // Operate a special action only when the message was sent from SingleMove plug-in.
    // SingleMoveプラグインからの通知のみ、特別な動作を行う
    if (src_product == 0x56A31D20 && src_id == 0x3901EA9B) {
        if (strcmp(description, "text") == 0) {
            swprintf_s(buf, len + 200, L"  from SingleMove : %S", (const char*)message);
            dlgMain->AddMessage(buf);
        }
    }

    delete[] buf;

    // Return 59 without a deep idea.
    // 適当に59を返している
    return 59;
}

//---------------------------------------------------------------------------
//  OnSubCommand
//    A message for calling a sub comand
//    サブコマンドの呼び出し
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::OnSubCommand(MQDocument doc, int index)
{
    switch (index) {
    case 0:
        dlgMain->m_Tab->SetCurrentPage(0);
        return FALSE;
    case 1:
        dlgMain->m_Tab->SetCurrentPage(1);
        return FALSE;
    case 2:
        dlgMain->m_Tab->SetCurrentPage(2);
        return FALSE;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
//  OnDraw
//    描画時の処理
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnDraw(MQDocument doc, MQScene scene, int width, int height)
{
    if (!m_bOnDraw)
        return;

    wchar_t buf[256];
    swprintf_s(buf, L"OnDraw : width=%d, height=%d", width, height);
    dlgMain->AddMessage(buf);
}


//---------------------------------------------------------------------------
//  OnNewDocument
//    ドキュメント初期化時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnNewDocument(MQDocument doc, const char *filename, NEW_DOCUMENT_PARAM& param)
{
    wchar_t buf[256 + MAX_PATH];
    swprintf_s(buf, L"OnNewDocument : filename=%S", (filename != NULL) ? filename : "(none)");
    dlgMain->AddMessage(buf);

    if (m_bActivate) {
        dlgMain->UpdateList(doc, false);
    }
}

//---------------------------------------------------------------------------
//  OnEndDocument
//    ドキュメント終了時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnEndDocument(MQDocument doc)
{
    dlgMain->AddMessage(L"OnEndDocument");
}

//---------------------------------------------------------------------------
//  OnSaveDocument
//    ドキュメント保存時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnSaveDocument(MQDocument doc, const char *filename, SAVE_DOCUMENT_PARAM& param)
{
    wchar_t buf[256 + MAX_PATH];
    swprintf_s(buf, L"OnSaveDocument : filename=%S", (filename != NULL) ? filename : "(none)");
    dlgMain->AddMessage(buf);
}

//---------------------------------------------------------------------------
//  OnUndo
//    アンドゥ実行時
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::OnUndo(MQDocument doc, int undo_state)
{
    wchar_t buf[256];
    swprintf_s(buf, L"OnUndo : undo_state=%d", undo_state);
    dlgMain->AddMessage(buf);

    if (m_bActivate) {
        dlgMain->UpdateList(doc, false);
    }

    return FALSE;
}

//---------------------------------------------------------------------------
//  OnRedo
//    リドゥ実行時
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::OnRedo(MQDocument doc, int redo_state)
{
    wchar_t buf[256];
    swprintf_s(buf, L"OnRedo : redo_state=%d", redo_state);
    dlgMain->AddMessage(buf);

    if (m_bActivate) {
        dlgMain->UpdateList(doc, false);
    }

    return FALSE;
}

//---------------------------------------------------------------------------
//  OnUpdateUndo
//    アンドゥ状態更新時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnUpdateUndo(MQDocument doc, int undo_state, int undo_size)
{
    if (!m_bOnUpdateUndo)
        return;

    wchar_t buf[256];
    swprintf_s(buf, L"OnUpdateUndo : undo_state=%d undo_size=%d", undo_state, undo_size);
    dlgMain->AddMessage(buf);
}

//---------------------------------------------------------------------------
//  OnObjectModified
//    オブジェクトの編集時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnObjectModified(MQDocument doc)
{
    dlgMain->AddMessage(L"OnObjectModified");

    if (m_bActivate) {
        dlgMain->UpdateList(doc, false);
    }
}

//---------------------------------------------------------------------------
//  OnObjectSelected
//    オブジェクトの選択状態の変更時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnObjectSelected(MQDocument doc)
{
    dlgMain->AddMessage(L"OnObjectSelected");

    if (m_bActivate) {
        dlgMain->UpdateList(doc, true);
    }
}

//---------------------------------------------------------------------------
//  OnUpdateObjectList
//    カレントオブジェクトの変更時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnUpdateObjectList(MQDocument doc)
{
    dlgMain->AddMessage(L"OnUpdateObjectList");

    if (m_bActivate) {
        dlgMain->UpdateList(doc, false);
    }
}

//---------------------------------------------------------------------------
//  OnMaterialModified
//    マテリアルのパラメータ変更時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnMaterialModified(MQDocument doc)
{
    dlgMain->AddMessage(L"OnMaterialModified");
}

//---------------------------------------------------------------------------
//  OnUpdateMaterialList
//    カレントマテリアルの変更時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnUpdateMaterialList(MQDocument doc)
{
    dlgMain->AddMessage(L"OnUpdateMaterialList");
}

//---------------------------------------------------------------------------
//  OnUpdateScene
//    シーン情報の変更時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnUpdateScene(MQDocument doc, MQScene scene)
{
    if (!m_bOnUpdateScene)
        return;

    dlgMain->AddMessage(L"OnUpdateScene");
}

//---------------------------------------------------------------------------
//  ExecuteCallback
//    コールバックに対する実装部
//---------------------------------------------------------------------------
bool MeshSyncClientPlugin::ExecuteCallback(MQDocument doc, void *option)
{
    CallbackInfo *info = (CallbackInfo*)option;
    return ((*this).*info->proc)(doc);
}

// コールバックの呼び出し
void MeshSyncClientPlugin::Execute(ExecuteCallbackProc proc)
{
    CallbackInfo info;
    info.proc = proc;
    BeginCallback(&info);
}

// プラグインのベースクラスを返す
MQBasePlugin *GetPluginClass()
{
    return &g_plugin;
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

        return SUCCEEDED(hRes);
    }

    if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        ::CoUninitialize();
    }

    return TRUE;
}

