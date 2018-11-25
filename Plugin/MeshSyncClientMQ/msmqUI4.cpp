#include "pch.h"
#include "msmqUI4.h"


SettingsDlg::SettingsDlg(MeshSyncClientPlugin *plugin, MQWindowBase& parent) : MQWindow(parent)
{
    setlocale(LC_ALL, "");

    m_plugin = plugin;

    SetTitle(L"Unity Mesh Sync");
    SetOutSpace(0.4);

    const size_t buf_len = 128;
    wchar_t buf[buf_len];
    double margin = 0.3;
    auto& s = getSettings();
    {
        MQFrame *vf = CreateVerticalFrame(this);
        vf->SetOutSpace(margin);

        MQFrame *hf = CreateHorizontalFrame(vf);
        CreateLabel(hf, L"Server : Port");
        m_edit_server = CreateEdit(hf);
        m_edit_server->SetText(ToWCS(s.client_settings.server));
        m_edit_server->AddChangedEvent(this, &SettingsDlg::OnServerChange);

        swprintf(buf, buf_len, L"%d", (int)s.client_settings.port);
        m_edit_port = CreateEdit(hf);
        m_edit_port->SetNumeric(MQEdit::NUMERIC_INT);
        m_edit_port->SetText(buf);
        m_edit_port->AddChangedEvent(this, &SettingsDlg::OnPortChange);

        swprintf(buf, buf_len, L"%.3f", s.scale_factor);
        hf = CreateHorizontalFrame(vf);
        CreateLabel(hf, L"Scale Factor");
        m_edit_scale = CreateEdit(hf);
        m_edit_scale->SetNumeric(MQEdit::NUMERIC_DOUBLE);
        m_edit_scale->SetText(buf);
        m_edit_scale->AddChangedEvent(this, &SettingsDlg::OnScaleChange);
    }

    {
        MQLabel *space = nullptr;
        MQFrame *vf = CreateVerticalFrame(this);
        vf->SetOutSpace(margin);

#if MQPLUGIN_VERSION >= 0x0460
        m_check_normals = CreateCheckBox(vf, L"Sync Normals");
        m_check_normals->SetChecked(s.sync_normals);
        m_check_normals->AddChangedEvent(this, &SettingsDlg::OnSyncNormalsChange);
#endif

        m_check_vcolor = CreateCheckBox(vf, L"Sync Vertex Color");
        m_check_vcolor->SetChecked(s.sync_vertex_color);
        m_check_vcolor->AddChangedEvent(this, &SettingsDlg::OnSyncVertexColorChange);

        m_check_double_sided = CreateCheckBox(vf, L"Double Sided");
        m_check_double_sided->SetChecked(s.make_double_sided);
        m_check_double_sided->AddChangedEvent(this, &SettingsDlg::OnmakeDoubleSidedChange);

#if MQPLUGIN_VERSION >= 0x0464
        m_check_bones = CreateCheckBox(vf, L"Sync Bones");
        m_check_bones->SetChecked(s.sync_bones);
        m_check_bones->AddChangedEvent(this, &SettingsDlg::OnSyncBonesChange);

        m_frame_poses = CreateHorizontalFrame(vf);
        space = CreateLabel(m_frame_poses, L" ");
        space->SetWidth(32);
        m_check_poses = CreateCheckBox(m_frame_poses, L"Sync Poses");
        m_check_poses->SetChecked(s.sync_poses);
        m_check_poses->AddChangedEvent(this, &SettingsDlg::OnSyncPosesChange);
#endif

        m_check_textures = CreateCheckBox(vf, L"Sync Textures");
        m_check_textures->SetChecked(s.sync_textures);
        m_check_textures->AddChangedEvent(this, &SettingsDlg::OnSyncTexturesChange);

        m_check_camera = CreateCheckBox(vf, L"Sync Camera");
        m_check_camera->SetChecked(s.sync_camera);
        m_check_camera->AddChangedEvent(this, &SettingsDlg::OnSyncCameraChange);

        m_frame_camera_path = CreateHorizontalFrame(vf);
        space = CreateLabel(m_frame_camera_path, L" ");
        space->SetWidth(32);
        CreateLabel(m_frame_camera_path, L"Camera Path");
        m_edit_camera_path = CreateEdit(m_frame_camera_path);
        m_edit_camera_path->SetText(ToWCS(s.host_camera_path));
        m_edit_camera_path->AddChangedEvent(this, &SettingsDlg::OnCameraPathChange);
        m_edit_camera_path->SetHorzLayout(LAYOUT_FILL);
        m_frame_camera_path->SetVisible(m_check_camera->GetChecked());
    }

    {
        MQFrame *vf = CreateVerticalFrame(this);
        vf->SetOutSpace(margin);

        m_check_autosync = CreateCheckBox(vf, L"Auto Sync");
        m_check_autosync->SetChecked(s.auto_sync);
        m_check_autosync->AddChangedEvent(this, &SettingsDlg::OnAutoSyncChange);

        m_button_sync = CreateButton(vf, L"Manual Sync");
        m_button_sync->AddClickEvent(this, &SettingsDlg::OnSyncClicked);
    }

    {
        MQFrame *vf = CreateVerticalFrame(this);
        vf->SetOutSpace(margin);

        CreateLabel(vf, L"Import Settings");

        m_check_bake_skin = CreateCheckBox(vf, L"Bake Skin");
        m_check_bake_skin->SetChecked(s.bake_skin);
        m_check_bake_skin->AddChangedEvent(this, &SettingsDlg::OnBakeSkinChange);

        m_check_bake_cloth = CreateCheckBox(vf, L"Bake Cloth");
        m_check_bake_cloth->SetChecked(s.bake_cloth);
        m_check_bake_cloth->AddChangedEvent(this, &SettingsDlg::OnBakeClothChange);

        m_button_import = CreateButton(vf, L"Import Unity Scene");
        m_button_import->AddClickEvent(this, &SettingsDlg::OnImportClicked);
    }

    {
        MQFrame *vf = CreateVerticalFrame(this);

        std::string plugin_version = "Plugin Version: " msReleaseDateStr;
        CreateLabel(vf, ToWCS(plugin_version));
    }

    this->AddHideEvent(this, &SettingsDlg::OnHide);
}

msmqSettings& SettingsDlg::getSettings()
{
    return m_plugin->getContext().getSettings();
}


BOOL SettingsDlg::OnHide(MQWidgetBase *sender, MQDocument doc)
{
    m_plugin->WindowClose();
    return FALSE;
}

BOOL SettingsDlg::OnServerChange(MQWidgetBase * sender, MQDocument doc)
{
    auto v = ToMBS(m_edit_server->GetText());
    getSettings().client_settings.server = v;
    return 0;
}

BOOL SettingsDlg::OnPortChange(MQWidgetBase * sender, MQDocument doc)
{
    auto v = ToMBS(m_edit_port->GetText());
    getSettings().client_settings.server = std::atoi(v.c_str());
    return 0;
}

BOOL SettingsDlg::OnScaleChange(MQWidgetBase * sender, MQDocument doc)
{
    auto v = ToMBS(m_edit_scale->GetText());
    auto f = std::atof(v.c_str());
    if (f != 0.0) {
        getSettings().scale_factor = (float)f;
        m_plugin->SendAll(true);
    }
    return 0;
}

BOOL SettingsDlg::OnSyncNormalsChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().sync_normals = m_check_normals->GetChecked();
    m_plugin->SendAll(true);
    return 0;
}

BOOL SettingsDlg::OnSyncVertexColorChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().sync_vertex_color = m_check_vcolor->GetChecked();
    m_plugin->SendAll(true);
    return 0;
}

BOOL SettingsDlg::OnmakeDoubleSidedChange(MQWidgetBase * sender, MQDocument doc)
{
    getSettings().make_double_sided = m_check_double_sided->GetChecked();
    m_plugin->SendAll(true);
    return 0;
}

BOOL SettingsDlg::OnSyncBonesChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().sync_bones = m_check_bones->GetChecked();
    m_frame_poses->SetVisible(m_check_bones->GetChecked());
    m_plugin->SendAll(true);
    return 0;
}

BOOL SettingsDlg::OnSyncPosesChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().sync_poses = m_check_poses->GetChecked();
    m_plugin->SendAll(true);
    return 0;
}

BOOL SettingsDlg::OnSyncTexturesChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().sync_textures = m_check_textures->GetChecked();
    m_plugin->SendAll(true);
    return 0;
}

BOOL SettingsDlg::OnSyncCameraChange(MQWidgetBase * sender, MQDocument doc)
{
    bool checked = m_check_camera->GetChecked();
    getSettings().sync_camera = checked;
    m_frame_camera_path->SetVisible(checked);
    if (checked) {
        m_plugin->SendCamera(true);
    }
    return 0;
}

BOOL SettingsDlg::OnCameraPathChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().host_camera_path = ToMBS(m_edit_camera_path->GetText());
    return 0;
}

BOOL SettingsDlg::OnAutoSyncChange(MQWidgetBase * sender, MQDocument doc)
{
    getSettings().auto_sync = m_check_autosync->GetChecked();
    m_plugin->SendAll(true);
    return 0;
}

BOOL SettingsDlg::OnSyncClicked(MQWidgetBase * sender, MQDocument doc)
{
    m_plugin->SendAll(false);
    return 0;
}


BOOL SettingsDlg::OnBakeSkinChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().bake_skin = m_check_bake_skin->GetChecked();
    return 0;
}
BOOL SettingsDlg::OnBakeClothChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().bake_cloth = m_check_bake_cloth->GetChecked();
    return 0;
}

BOOL SettingsDlg::OnImportClicked(MQWidgetBase * sender, MQDocument doc)
{
    m_plugin->Import();
    return 0;
}
