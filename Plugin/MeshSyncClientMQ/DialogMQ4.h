#pragma once

#include "MQWidget.h"

class MeshSyncClientPlugin;

class SettingsDlg : public MQWindow
{
public:
    SettingsDlg(MeshSyncClientPlugin *plugin, MQWindowBase& parent);

    BOOL OnHide(MQWidgetBase *sender, MQDocument doc);
    BOOL OnServerChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnPortChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnScaleChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnSyncCameraChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnAutoSyncChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnSyncClicked(MQWidgetBase *sender, MQDocument doc);
    BOOL OnBakeSkinChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnBakeClothChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnImportClicked(MQWidgetBase *sender, MQDocument doc);

private:
    MQEdit *m_edit_server = nullptr;
    MQEdit *m_edit_port = nullptr;
    MQEdit *m_edit_scale = nullptr;
    MQCheckBox *m_check_camera = nullptr;
    MQCheckBox *m_check_autosync = nullptr;
    MQButton *m_button_sync = nullptr;
    MQCheckBox *m_check_bake_skin = nullptr;
    MQCheckBox *m_check_bake_cloth = nullptr;
    MQButton *m_button_import = nullptr;

    MeshSyncClientPlugin *m_plugin = nullptr;
};
