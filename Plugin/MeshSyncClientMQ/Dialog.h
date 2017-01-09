#pragma once

#include "MQWidget.h"

class MeshSyncClientPlugin;

class SettingsDlg : public MQWindow
{
public:
    MQTab *m_Tab;
    MQListBox *m_MessageList;
    MQCheckBox *m_CheckOnDraw;
    MQCheckBox *m_CheckOnUpdateScene;
    MQCheckBox *m_CheckOnUpdateUndo;
    MQCheckBox *m_CheckWidgetEvent;
    MQCheckBox *m_CheckMouseMove;

    MQListBox *m_VertexList;
    MQListBox *m_FaceList;

    MeshSyncClientPlugin *m_pPlugin;

    SettingsDlg(MeshSyncClientPlugin *plugin, MQWindowBase& parent);

    BOOL OnTabChange(MQWidgetBase *sender, MQDocument doc);
    BOOL OnHide(MQWidgetBase *sender, MQDocument doc);
    BOOL OnAppAbout(MQWidgetBase *sender, MQDocument doc);
    BOOL OnOK(MQWidgetBase *sender, MQDocument doc);
    BOOL OnClear(MQWidgetBase *sender, MQDocument doc);
    BOOL OnCheckOnDraw(MQWidgetBase *sender, MQDocument doc);
    BOOL OnCheckOnUpdateScene(MQWidgetBase *sender, MQDocument doc);
    BOOL OnCheckOnUpdateUndo(MQWidgetBase *sender, MQDocument doc);
    BOOL OnCheckWidgetEvent(MQWidgetBase *sender, MQDocument doc);
    BOOL OnCheckMouseMove(MQWidgetBase *sender, MQDocument doc);

    BOOL OnListLeftDown(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListLeftUp(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListLeftDoubleClick(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListMiddleDown(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListMiddleUp(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListMiddleDoubleClick(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListRightDown(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListRightUp(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListRightDoubleClick(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListMouseMove(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListMouseWheel(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param);
    BOOL OnListKeyDown(MQWidgetBase *sender, MQDocument doc, MQWidgetKeyParam& param);
    BOOL OnListKeyUp(MQWidgetBase *sender, MQDocument doc, MQWidgetKeyParam& param);

    BOOL OnVertexSelect(MQWidgetBase *sender, MQDocument doc);
    BOOL OnFaceSelect(MQWidgetBase *sender, MQDocument doc);

    void AddMessage(const wchar_t *message);

    void UpdateList(MQDocument doc, bool select_only);
    void UpdateVertexList(MQDocument doc, bool select_only);
    void UpdateFaceList(MQDocument doc, bool select_only);
};
