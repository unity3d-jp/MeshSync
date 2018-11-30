#include "pch.h"
#include "MeshSyncClientMQ4.h"
#include "msmqUI4.h"

static MeshSyncClientPlugin g_plugin;

// Constructor
// コンストラクタ
MeshSyncClientPlugin::MeshSyncClientPlugin()
    : m_sync(this)
{
}

// Destructor
// デストラクタ
MeshSyncClientPlugin::~MeshSyncClientPlugin()
{
}

#if defined(__APPLE__) || defined(__linux__)
// Create a new plugin class for another document.
// 別のドキュメントのための新しいプラグインクラスを作成する。
MQBasePlugin* MeshSyncClientPlugin::CreateNewPlugin()
{
    return new MeshSyncClientPlugin();
}
#endif

//---------------------------------------------------------------------------
//  GetPlugInID
//    プラグインIDを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::GetPlugInID(DWORD *Product, DWORD *ID)
{
    // プロダクト名(制作者名)とIDを、全部で64bitの値として返す
    // 値は他と重複しないようなランダムなもので良い
    *Product = MQPluginProduct;
    *ID = MQPluginID;
}

//---------------------------------------------------------------------------
//  GetPlugInName
//    プラグイン名を返す。
//    この関数は[プラグインについて]表示時に呼び出される。
//---------------------------------------------------------------------------
const char *MeshSyncClientPlugin::GetPlugInName(void)
{
    return "Unity Mesh Sync (Release " msReleaseDateStr ")  Copyright(C) 2017-2018, Unity Technologies";
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
    return NULL;
}

//---------------------------------------------------------------------------
//  GetSubCommandString
//    サブコマンドの文字列を列挙
//---------------------------------------------------------------------------
const wchar_t *MeshSyncClientPlugin::GetSubCommandString(int index)
{
    return NULL;
}

//---------------------------------------------------------------------------
//  Initialize
//    アプリケーションの初期化
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::Initialize()
{
    if (!m_dlg) {
        auto parent = MQWindow::GetMainWindow();
        m_dlg = new SettingsDlg(this, parent);
    }
    return TRUE;
}

//---------------------------------------------------------------------------
//  Exit
//    アプリケーションの終了
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::Exit()
{
    if (m_dlg) {
        delete m_dlg;
        m_dlg = nullptr;
    }
}

//---------------------------------------------------------------------------
//  Activate
//    表示・非表示切り替え要求
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::Activate(MQDocument doc, BOOL flag)
{
    if (!m_dlg) {
        return FALSE;
    }

    bool active = flag ? true : false;
    m_dlg->SetVisible(active);
    return active;
}

//---------------------------------------------------------------------------
//  IsActivated
//    表示・非表示状態の返答
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::IsActivated(MQDocument doc)
{
    if (!m_dlg) {
        return FALSE;
    }
    return m_dlg->GetVisible();
}

//---------------------------------------------------------------------------
//  OnMinimize
//    ウインドウの最小化への返答
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnMinimize(MQDocument doc, BOOL flag)
{
}

//---------------------------------------------------------------------------
//  OnReceiveUserMessage
//    プラグイン独自のメッセージの受け取り
//---------------------------------------------------------------------------
int MeshSyncClientPlugin::OnReceiveUserMessage(MQDocument doc, DWORD src_product, DWORD src_id, const char *description, void *message)
{
    return 0;
}

//---------------------------------------------------------------------------
//  OnSubCommand
//    A message for calling a sub comand
//    サブコマンドの呼び出し
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::OnSubCommand(MQDocument doc, int index)
{
    return FALSE;
}

//---------------------------------------------------------------------------
//  OnDraw
//    描画時の処理
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnDraw(MQDocument doc, MQScene scene, int width, int height)
{
    m_sync.flushPendingRequests(doc);
    m_sync.sendCamera(doc);
}


//---------------------------------------------------------------------------
//  OnNewDocument
//    ドキュメント初期化時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnNewDocument(MQDocument doc, const char *filename, NEW_DOCUMENT_PARAM& param)
{
    m_sync.sendMeshes(doc);
}

//---------------------------------------------------------------------------
//  OnEndDocument
//    ドキュメント終了時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnEndDocument(MQDocument doc)
{
    m_sync.clear();
}

//---------------------------------------------------------------------------
//  OnSaveDocument
//    ドキュメント保存時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnSaveDocument(MQDocument doc, const char *filename, SAVE_DOCUMENT_PARAM& param)
{
}

//---------------------------------------------------------------------------
//  OnUndo
//    アンドゥ実行時
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::OnUndo(MQDocument doc, int undo_state)
{
    m_sync.sendMeshes(doc);
    return TRUE;
}

//---------------------------------------------------------------------------
//  OnRedo
//    リドゥ実行時
//---------------------------------------------------------------------------
BOOL MeshSyncClientPlugin::OnRedo(MQDocument doc, int redo_state)
{
    m_sync.sendMeshes(doc);
    return TRUE;
}

//---------------------------------------------------------------------------
//  OnUpdateUndo
//    アンドゥ状態更新時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnUpdateUndo(MQDocument doc, int undo_state, int undo_size)
{
    m_sync.sendMeshes(doc);
}

//---------------------------------------------------------------------------
//  OnObjectModified
//    オブジェクトの編集時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnObjectModified(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnObjectSelected
//    オブジェクトの選択状態の変更時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnObjectSelected(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnUpdateObjectList
//    カレントオブジェクトの変更時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnUpdateObjectList(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnMaterialModified
//    マテリアルのパラメータ変更時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnMaterialModified(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnUpdateMaterialList
//    カレントマテリアルの変更時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnUpdateMaterialList(MQDocument doc)
{
}

//---------------------------------------------------------------------------
//  OnUpdateScene
//    シーン情報の変更時
//---------------------------------------------------------------------------
void MeshSyncClientPlugin::OnUpdateScene(MQDocument doc, MQScene scene)
{
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

msmqContext& MeshSyncClientPlugin::getContext()
{
    return m_sync;
}

void MeshSyncClientPlugin::SendAll(bool only_when_autosync)
{
    if (!only_when_autosync || m_sync.getSettings().auto_sync)
        Execute(&MeshSyncClientPlugin::SendAllImpl);
}

void MeshSyncClientPlugin::SendCamera(bool only_when_autosync)
{
    if (!only_when_autosync || m_sync.getSettings().auto_sync)
        Execute(&MeshSyncClientPlugin::SendCameraImpl);
}

void MeshSyncClientPlugin::Import()
{
    Execute(&MeshSyncClientPlugin::ImportImpl);
}

bool MeshSyncClientPlugin::SendAllImpl(MQDocument doc)
{
    m_sync.sendMeshes(doc, true);
    m_sync.sendCamera(doc, true);
    return true;
}

bool MeshSyncClientPlugin::SendCameraImpl(MQDocument doc)
{
    m_sync.sendCamera(doc, true);
    return true;
}

bool MeshSyncClientPlugin::ImportImpl(MQDocument doc)
{
    m_sync.importMeshes(doc);
    return true;
}


#ifdef _WIN32
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
#endif
