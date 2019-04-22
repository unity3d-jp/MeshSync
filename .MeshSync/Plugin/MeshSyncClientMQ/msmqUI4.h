#pragma once

#include "MQWidget.h"
#include "MeshSyncClientMQ4.h"

class MeshSyncClientPlugin;

class msmqSettingsDlg : public MQWindow
{
public:
    msmqSettingsDlg(MeshSyncClientPlugin *plugin, MQWindowBase& parent);

    BOOL OnHide(MQWidgetBase *sender, MQDocument doc);
    BOOL OnServerChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnPortChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnScaleChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnSyncNormalsChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnSyncVertexColorChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnmakeDoubleSidedChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnSyncBonesChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnSyncPosesChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnSyncMorphsChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnSyncTexturesChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnSyncCameraChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnCameraPathChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnAutoSyncChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnSyncClicked(MQWidgetBase *sender, MQDocument doc);

    BOOL OnBakeSkinChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnBakeClothChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnImportClicked(MQWidgetBase *sender, MQDocument doc);

    void LogInfo(const char *message);

private:
    msmqSettings& getSettings();

    MQEdit *m_edit_server = nullptr;
    MQEdit *m_edit_port = nullptr;
    MQEdit *m_edit_scale = nullptr;
    MQCheckBox *m_check_normals = nullptr;
    MQCheckBox *m_check_vcolor = nullptr;
    MQCheckBox *m_check_double_sided = nullptr;

    MQCheckBox *m_check_bones = nullptr;
    MQFrame *m_frame_poses = nullptr;
    MQCheckBox *m_check_poses = nullptr;
    MQCheckBox *m_check_morphs = nullptr;

    MQCheckBox *m_check_textures = nullptr;

    MQFrame *m_frame_camera_path = nullptr;
    MQCheckBox *m_check_camera = nullptr;
    MQEdit *m_edit_camera_path = nullptr;

    MQCheckBox *m_check_autosync = nullptr;
    MQButton *m_button_sync = nullptr;
    MQCheckBox *m_check_bake_skin = nullptr;
    MQCheckBox *m_check_bake_cloth = nullptr;
    MQButton *m_button_import = nullptr;

    MQLabel *m_log = nullptr;

    MeshSyncClientPlugin *m_plugin = nullptr;
};
