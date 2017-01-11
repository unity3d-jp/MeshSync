#include "pch.h"
#include "DialogMQ4.h"
#include "MeshSyncClientMQ4.h"


SettingsDlg::SettingsDlg(MeshSyncClientPlugin *plugin, MQWindowBase& parent) : MQWindow(parent)
{
    setlocale(LC_ALL, "");

    m_pPlugin = plugin;
    /*
    // Set icons
    HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME),
    IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
    SetIcon(hIcon, TRUE);
    HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME),
    IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    SetIcon(hIconSmall, FALSE);
    */

    SetTitle(L"Station Spy");
    SetOutSpace(0.4);

    m_Tab = CreateTab(this);
    m_Tab->AddChangedEvent(this, &SettingsDlg::OnTabChange);
    m_Tab->SetVertLayout(MQWidgetBase::LAYOUT_FILL);

    MQFrame *mainFrame = CreateHorizontalFrame(m_Tab);
    mainFrame->SetVertLayout(MQWidgetBase::LAYOUT_FILL);

    m_MessageList = CreateListBox(mainFrame);
    m_MessageList->SetHorzLayout(MQWidgetBase::LAYOUT_FILL);
    m_MessageList->SetHintSizeRateX(20.0);

    MQFrame *sideFrame = CreateVerticalFrame(mainFrame);
    MQButton *okbtn = CreateButton(sideFrame, L"OK");
    okbtn->AddClickEvent(this, &SettingsDlg::OnOK);
    MQButton *clearbtn = CreateButton(sideFrame, L"Clear");
    clearbtn->AddClickEvent(this, &SettingsDlg::OnClear);

    m_CheckOnDraw = CreateCheckBox(sideFrame, L"OnDraw");
    m_CheckOnDraw->AddChangedEvent(this, &SettingsDlg::OnCheckOnDraw);

    m_CheckOnUpdateScene = CreateCheckBox(sideFrame, L"OnUpdateScene");
    m_CheckOnUpdateScene->AddChangedEvent(this, &SettingsDlg::OnCheckOnUpdateScene);

    m_CheckOnUpdateUndo = CreateCheckBox(sideFrame, L"OnUpdateUndo");
    m_CheckOnUpdateUndo->AddChangedEvent(this, &SettingsDlg::OnCheckOnUpdateUndo);

    m_CheckWidgetEvent = CreateCheckBox(sideFrame, L"Widget events");
    m_CheckWidgetEvent->AddChangedEvent(this, &SettingsDlg::OnCheckWidgetEvent);

    m_CheckMouseMove = CreateCheckBox(sideFrame, L"MouseMove");
    m_CheckMouseMove->AddChangedEvent(this, &SettingsDlg::OnCheckMouseMove);

    MQButton *aboutbtn = CreateButton(sideFrame, L"About");
    aboutbtn->AddClickEvent(this, &SettingsDlg::OnAppAbout);
    aboutbtn->SetFillBeforeRate(1);

    m_Tab->SetTabTitle(0, L"Message");

    mainFrame = CreateHorizontalFrame(m_Tab);
    mainFrame->SetVertLayout(MQWidgetBase::LAYOUT_FILL);
    m_Tab->SetTabTitle(1, L"Vertex");

    m_VertexList = CreateListBox(mainFrame);
    m_VertexList->SetMultiSelect(true);
    m_VertexList->SetHorzLayout(MQWidgetBase::LAYOUT_FILL);
    m_VertexList->SetHintSizeRateX(20.0);

    sideFrame = CreateVerticalFrame(mainFrame);
    okbtn = CreateButton(sideFrame, L"Select");
    okbtn->AddClickEvent(this, &SettingsDlg::OnVertexSelect);

    mainFrame = CreateHorizontalFrame(m_Tab);
    mainFrame->SetVertLayout(MQWidgetBase::LAYOUT_FILL);
    m_Tab->SetTabTitle(2, L"Face");

    m_FaceList = CreateListBox(mainFrame);
    m_FaceList->SetMultiSelect(true);
    m_FaceList->SetHorzLayout(MQWidgetBase::LAYOUT_FILL);
    m_FaceList->SetHintSizeRateX(20.0);

    sideFrame = CreateVerticalFrame(mainFrame);
    okbtn = CreateButton(sideFrame, L"Select");
    okbtn->AddClickEvent(this, &SettingsDlg::OnFaceSelect);

    this->AddHideEvent(this, &SettingsDlg::OnHide);
}

BOOL SettingsDlg::OnHide(MQWidgetBase *sender, MQDocument doc)
{
    return FALSE;
}

BOOL SettingsDlg::OnTabChange(MQWidgetBase *sender, MQDocument doc)
{
    UpdateList(doc, false);
    return FALSE;
}

BOOL SettingsDlg::OnAppAbout(MQWidgetBase *sender, MQDocument doc)
{
    //CAboutDlg dlg(*this);
    //dlg.Execute();
    return FALSE;
}

BOOL SettingsDlg::OnOK(MQWidgetBase *sender, MQDocument doc)
{
    SetVisible(false);

    return FALSE;
}

BOOL SettingsDlg::OnClear(MQWidgetBase *sender, MQDocument doc)
{
    m_MessageList->ClearItems();
    return FALSE;
}

BOOL SettingsDlg::OnCheckOnDraw(MQWidgetBase *sender, MQDocument doc)
{
    return FALSE;
}

BOOL SettingsDlg::OnCheckOnUpdateScene(MQWidgetBase *sender, MQDocument doc)
{
    return FALSE;
}

BOOL SettingsDlg::OnCheckOnUpdateUndo(MQWidgetBase *sender, MQDocument doc)
{
    return FALSE;
}

BOOL SettingsDlg::OnCheckWidgetEvent(MQWidgetBase *sender, MQDocument doc)
{
    if (false) {
        m_MessageList->AddLeftDownEvent(this, &SettingsDlg::OnListLeftDown);
        m_MessageList->AddLeftUpEvent(this, &SettingsDlg::OnListLeftUp);
        m_MessageList->AddLeftDoubleClickEvent(this, &SettingsDlg::OnListLeftDoubleClick);
        m_MessageList->AddMiddleDownEvent(this, &SettingsDlg::OnListMiddleDown);
        m_MessageList->AddMiddleUpEvent(this, &SettingsDlg::OnListMiddleUp);
        m_MessageList->AddMiddleDoubleClickEvent(this, &SettingsDlg::OnListMiddleDoubleClick);
        m_MessageList->AddRightDownEvent(this, &SettingsDlg::OnListRightDown);
        m_MessageList->AddRightUpEvent(this, &SettingsDlg::OnListRightUp);
        m_MessageList->AddRightDoubleClickEvent(this, &SettingsDlg::OnListRightDoubleClick);
        m_MessageList->AddMouseWheelEvent(this, &SettingsDlg::OnListMouseWheel);
        m_MessageList->AddKeyDownEvent(this, &SettingsDlg::OnListKeyDown);
        m_MessageList->AddKeyUpEvent(this, &SettingsDlg::OnListKeyUp);
    }
    else {
        m_MessageList->RemoveLeftDownEvent(this, &SettingsDlg::OnListLeftDown);
        m_MessageList->RemoveLeftUpEvent(this, &SettingsDlg::OnListLeftUp);
        m_MessageList->RemoveLeftDoubleClickEvent(this, &SettingsDlg::OnListLeftDoubleClick);
        m_MessageList->RemoveMiddleDownEvent(this, &SettingsDlg::OnListMiddleDown);
        m_MessageList->RemoveMiddleUpEvent(this, &SettingsDlg::OnListMiddleUp);
        m_MessageList->RemoveMiddleDoubleClickEvent(this, &SettingsDlg::OnListMiddleDoubleClick);
        m_MessageList->RemoveRightDownEvent(this, &SettingsDlg::OnListRightDown);
        m_MessageList->RemoveRightUpEvent(this, &SettingsDlg::OnListRightUp);
        m_MessageList->RemoveRightDoubleClickEvent(this, &SettingsDlg::OnListRightDoubleClick);
        m_MessageList->RemoveMouseWheelEvent(this, &SettingsDlg::OnListMouseWheel);
        m_MessageList->RemoveKeyDownEvent(this, &SettingsDlg::OnListKeyDown);
        m_MessageList->RemoveKeyUpEvent(this, &SettingsDlg::OnListKeyUp);
    }

    return FALSE;
}

BOOL SettingsDlg::OnCheckMouseMove(MQWidgetBase *sender, MQDocument doc)
{
    if (false) {
        m_MessageList->AddMouseMoveEvent(this, &SettingsDlg::OnListMouseMove);
    }
    else {
        m_MessageList->RemoveMouseMoveEvent(this, &SettingsDlg::OnListMouseMove);
    }

    return TRUE;
}

BOOL SettingsDlg::OnListLeftDown(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"LeftDown : pos(%d %d)", param.ClientPos.x, param.ClientPos.y);
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListLeftUp(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"LeftUp : pos(%d %d)", param.ClientPos.x, param.ClientPos.y);
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListLeftDoubleClick(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"LeftDoubleClick : pos(%d %d)", param.ClientPos.x, param.ClientPos.y);
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListMiddleDown(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"MiddleDown : pos(%d %d)", param.ClientPos.x, param.ClientPos.y);
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListMiddleUp(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"MiddleUp : pos(%d %d)", param.ClientPos.x, param.ClientPos.y);
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListMiddleDoubleClick(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"MiddleDoubleClick : pos(%d %d)", param.ClientPos.x, param.ClientPos.y);
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListRightDown(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"RightDown : pos(%d %d)", param.ClientPos.x, param.ClientPos.y);
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListRightUp(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"RightUp : pos(%d %d)", param.ClientPos.x, param.ClientPos.y);
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListRightDoubleClick(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"RightDoubleClick : pos(%d %d)", param.ClientPos.x, param.ClientPos.y);
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListMouseMove(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"MouseMove : pos(%d %d) %s%s%s", param.ClientPos.x, param.ClientPos.y,
        param.LButton ? L"L" : L"", param.MButton ? L"M" : L"", param.RButton ? L"R" : L"");
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListMouseWheel(MQWidgetBase *sender, MQDocument doc, MQWidgetMouseParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"MouseWheel : %d", param.Wheel);
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListKeyDown(MQWidgetBase *sender, MQDocument doc, MQWidgetKeyParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"KeyDown : %d %s", param.Key, param.AutoRepeat ? L"repeat" : L"");
    AddMessage(buf);
    return FALSE;
}

BOOL SettingsDlg::OnListKeyUp(MQWidgetBase *sender, MQDocument doc, MQWidgetKeyParam& param)
{
    wchar_t buf[256];
    swprintf_s(buf, L"KeyUp : %d", param.Key);
    AddMessage(buf);
    return FALSE;
}

// Add a message log
// メッセージログの追加
void SettingsDlg::AddMessage(const wchar_t *message)
{
    int index = m_MessageList->AddItem(message);
    m_MessageList->MakeItemVisible(index);
}

void SettingsDlg::UpdateList(MQDocument doc, bool select_only)
{
    switch (m_Tab->GetCurrentPage()) {
    case 1:
        UpdateVertexList(doc, select_only);
        break;
    case 2:
        UpdateFaceList(doc, select_only);
        break;
    }
}

void SettingsDlg::UpdateVertexList(MQDocument doc, bool select_only)
{
    if (!select_only) {
        m_VertexList->ClearItems();
    }

    int curobj = doc->GetCurrentObjectIndex();
    if (curobj == -1) return;

    MQObject obj = doc->GetObject(curobj);
    if (obj == NULL) return;

    if (!select_only) {
        int num = obj->GetVertexCount();
        for (int i = 0; i<num; i++) {
            if (obj->GetVertexRefCount(i) == 0) continue;

            MQPoint p = obj->GetVertex(i);
            UINT uid = obj->GetVertexUniqueID(i);
            wchar_t buf[256];
            swprintf_s(buf, L"%d [id:%u] (%f %f %f)", i, uid, p.x, p.y, p.z);
            m_VertexList->AddItem(buf, i);

            if (doc->IsSelectVertex(curobj, i)) {
                m_VertexList->SetItemSelected(i, true);
            }
        }
    }
    else {
        int numVertex = obj->GetVertexCount();
        int num = m_VertexList->GetItemCount();
        for (int i = 0; i<num; i++) {
            int vert = (int)m_VertexList->GetItemTag(i);
            if (vert >= 0 && vert < numVertex) {
                m_VertexList->SetItemSelected(i, doc->IsSelectVertex(curobj, vert) != FALSE);
            }
        }
    }
}

void SettingsDlg::UpdateFaceList(MQDocument doc, bool select_only)
{
    if (!select_only) {
        m_FaceList->ClearItems();
    }

    int curobj = doc->GetCurrentObjectIndex();
    if (curobj == -1) return;

    MQObject obj = doc->GetObject(curobj);
    if (obj == NULL) return;

    if (!select_only) {
        int num = obj->GetFaceCount();
        for (int i = 0; i<num; i++) {
            int ptnum = obj->GetFacePointCount(i);
            if (ptnum == 0) continue;

            std::vector<int> ptarray(ptnum);
            obj->GetFacePointArray(i, &(*ptarray.begin()));

            UINT uid = obj->GetFaceUniqueID(i);

            std::wostringstream st;
            st << i << L" [id:" << uid << L"] (";
            for (int j = 0; j<ptnum; j++) {
                if (j > 0) st << L" ";
                st << ptarray[j];
            }
            st << L")";
            m_FaceList->AddItem(st.str(), i);

            if (doc->IsSelectFace(curobj, i)) {
                m_FaceList->SetItemSelected(i, true);
            }
        }
    }
    else {
        int numFace = obj->GetFaceCount();
        int num = m_FaceList->GetItemCount();
        for (int i = 0; i<num; i++) {
            int face = (int)m_FaceList->GetItemTag(i);
            if (face >= 0 && face < numFace) {
                m_FaceList->SetItemSelected(i, doc->IsSelectFace(curobj, face) != FALSE);
            }
        }
    }
}

BOOL SettingsDlg::OnVertexSelect(MQWidgetBase *sender, MQDocument doc)
{
    int curobj = doc->GetCurrentObjectIndex();
    if (curobj == -1) return FALSE;

    MQObject obj = doc->GetObject(curobj);
    if (obj == NULL) return FALSE;

    int numVertex = obj->GetVertexCount();
    int num = m_VertexList->GetItemCount();
    for (int i = 0; i<num; i++) {
        int vert = (int)m_VertexList->GetItemTag(i);
        if (vert >= 0 && vert < numVertex) {
            if (m_VertexList->GetItemSelected(i)) {
                doc->AddSelectVertex(curobj, vert);
            }
            else {
                doc->DeleteSelectVertex(curobj, vert);
            }
        }
    }

    return TRUE;
}

BOOL SettingsDlg::OnFaceSelect(MQWidgetBase *sender, MQDocument doc)
{
    int curobj = doc->GetCurrentObjectIndex();
    if (curobj == -1) return FALSE;

    MQObject obj = doc->GetObject(curobj);
    if (obj == NULL) return FALSE;

    int numFace = obj->GetFaceCount();
    int num = m_FaceList->GetItemCount();
    for (int i = 0; i<num; i++) {
        int face = (int)m_FaceList->GetItemTag(i);
        if (face >= 0 && face < numFace) {
            if (m_FaceList->GetItemSelected(i)) {
                doc->AddSelectFace(curobj, face);
            }
            else {
                doc->DeleteSelectFace(curobj, face);
            }
        }
    }

    return TRUE;
}

