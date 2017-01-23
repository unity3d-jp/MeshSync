#include "pch.h"
#include "DialogMQ4.h"
#include "MeshSyncClientMQ4.h"


static inline std::wstring L(const std::string& s)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(s);
}
static inline std::string S(const std::wstring& w)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(w);
}

SettingsDlg::SettingsDlg(MeshSyncClientPlugin *plugin, MQWindowBase& parent) : MQWindow(parent)
{
    setlocale(LC_ALL, "");

    m_plugin = plugin;

    SetTitle(L"Unity Mesh Sync");
    SetOutSpace(0.4);

    wchar_t buf[128];

    MQFrame *frame = CreateVerticalFrame(this);
    {
        MQFrame *hf = CreateHorizontalFrame(frame);
        CreateLabel(hf, L"Server : Port");
        m_edit_server = CreateEdit(hf);
        m_edit_server->SetText(L(m_plugin->getSync().getClientSettings().server));
        m_edit_server->AddChangedEvent(this, &SettingsDlg::OnServerChange);

        swprintf(buf, L"%d", (int)m_plugin->getSync().getClientSettings().port);
        m_edit_port = CreateEdit(hf);
        m_edit_port->SetNumeric(MQEdit::NUMERIC_INT);
        m_edit_port->SetText(buf);
        m_edit_port->AddChangedEvent(this, &SettingsDlg::OnPortChange);
    }
    {
        swprintf(buf, L"%.3f", m_plugin->getSync().getScaleFactor());
        MQFrame *hf = CreateHorizontalFrame(frame);
        CreateLabel(hf, L"Scale Factor");
        m_edit_scale = CreateEdit(hf);
        m_edit_scale->SetNumeric(MQEdit::NUMERIC_DOUBLE);
        m_edit_scale->SetText(buf);
        m_edit_scale->AddChangedEvent(this, &SettingsDlg::OnScaleChange);
    }

    m_check_autosync = CreateCheckBox(frame, L"Auto Sync");
    m_check_autosync->SetChecked(m_plugin->getSync().getAutoSync());
    m_check_autosync->AddChangedEvent(this, &SettingsDlg::OnAutoSyncChange);

    m_button_sync = CreateButton(frame, L"Manual Sync");
    m_button_sync->AddClickEvent(this, &SettingsDlg::OnSyncClicked);

    {
        MQFrame *vf = CreateVerticalFrame(this);

        CreateLabel(vf, L"Import Settings");

        m_check_bake_skin = CreateCheckBox(vf, L"Bake SKin");
        m_check_bake_skin->SetChecked(m_plugin->getSync().getBakeSkin());
        m_check_bake_skin->AddChangedEvent(this, &SettingsDlg::OnBakeSkinChange);

        m_check_bake_cloth = CreateCheckBox(vf, L"Bake Cloth");
        m_check_bake_cloth->SetChecked(m_plugin->getSync().getBakeCloth());
        m_check_bake_cloth->AddChangedEvent(this, &SettingsDlg::OnBakeClothChange);

        m_button_import = CreateButton(vf, L"Import Unity Scene");
        m_button_import->AddClickEvent(this, &SettingsDlg::OnImportClicked);
    }

    this->AddHideEvent(this, &SettingsDlg::OnHide);
}

BOOL SettingsDlg::OnHide(MQWidgetBase *sender, MQDocument doc)
{
    m_plugin->WindowClose();
    return FALSE;
}

BOOL SettingsDlg::OnServerChange(MQWidgetBase * sender, MQDocument doc)
{
    auto v = S(m_edit_server->GetText());
    m_plugin->getSync().getClientSettings().server = v;
    return 0;
}

BOOL SettingsDlg::OnPortChange(MQWidgetBase * sender, MQDocument doc)
{
    auto v = S(m_edit_port->GetText());
    m_plugin->getSync().getClientSettings().server = std::atoi(v.c_str());
    return 0;
}

BOOL SettingsDlg::OnScaleChange(MQWidgetBase * sender, MQDocument doc)
{
    auto v = S(m_edit_scale->GetText());
    auto f = std::atof(v.c_str());
    if (f != 0.0) {
        m_plugin->getSync().getScaleFactor() = (float)f;
    }
    return 0;
}

BOOL SettingsDlg::OnAutoSyncChange(MQWidgetBase * sender, MQDocument doc)
{
    m_plugin->getSync().getAutoSync() = m_check_autosync->GetChecked();
    return 0;
}

BOOL SettingsDlg::OnSyncClicked(MQWidgetBase * sender, MQDocument doc)
{
    m_plugin->Send();
    return 0;
}

BOOL SettingsDlg::OnBakeSkinChange(MQWidgetBase *sender, MQDocument doc)
{
    m_plugin->getSync().getBakeSkin() = m_check_bake_skin->GetChecked();
    return 0;
}
BOOL SettingsDlg::OnBakeClothChange(MQWidgetBase *sender, MQDocument doc)
{
    m_plugin->getSync().getBakeCloth() = m_check_bake_cloth->GetChecked();
    return 0;
}

BOOL SettingsDlg::OnImportClicked(MQWidgetBase * sender, MQDocument doc)
{
    m_plugin->Import();
    return 0;
}

