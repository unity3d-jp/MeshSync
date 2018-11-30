#pragma once

#include "msmqContext.h"

class MeshSyncClientPlugin : public MQStationPlugin
{
public:
    MeshSyncClientPlugin();
    ~MeshSyncClientPlugin() override;

    void GetPlugInID(DWORD *Product, DWORD *ID) override;
    const char *GetPlugInName(void) override;
    const char *EnumString(void) override;

    BOOL Initialize() override;
    void Exit() override;

    BOOL Activate(MQDocument doc, BOOL flag) override;
    BOOL IsActivated(MQDocument doc) override;
    void OnMinimize(MQDocument doc, BOOL flag) override;
    int OnReceiveUserMessage(MQDocument doc, DWORD src_product, DWORD src_id, const char *description, void *message) override;
    void OnDraw(MQDocument doc, MQScene scene, int width, int height) override;

    void OnNewDocument(MQDocument doc, const char *filename, NEW_DOCUMENT_PARAM& param) override;
    void OnEndDocument(MQDocument doc) override;
    BOOL OnUndo(MQDocument doc, int undo_state) override;
    BOOL OnRedo(MQDocument doc, int redo_state) override;
    void OnUpdateUndo(MQDocument doc, int undo_state, int undo_size) override;
    void OnObjectModified(MQDocument doc) override;
    void OnObjectSelected(MQDocument doc) override;
    void OnUpdateObjectList(MQDocument doc) override;
    void OnMaterialModified(MQDocument doc) override;
    void OnUpdateMaterialList(MQDocument doc) override;

    typedef bool (MeshSyncClientPlugin::*ExecuteCallbackProc)(MQDocument doc);
    void Execute(ExecuteCallbackProc proc);

    struct CallbackInfo {
        ExecuteCallbackProc proc;
    };
    bool ExecuteCallback(MQDocument doc, void *option) override;


    msmqContext& getContext();
    bool& getActive();

    void SendAll(bool only_when_autosync);
    void SendCamera(bool only_when_autosync);
    void Import();

private:
    bool SendAllImpl(MQDocument doc);
    bool SendCameraImpl(MQDocument doc);
    bool ImportImpl(MQDocument doc);

    msmqContext m_sync;
    bool m_active = false;
};
