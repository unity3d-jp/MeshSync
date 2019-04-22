#include "pch.h"
#include "msmqUI4.h"


msmqSettingsDlg::msmqSettingsDlg(MeshSyncClientPlugin *plugin, MQWindowBase& parent) : MQWindow(parent)
{
    setlocale(LC_ALL, "");

    m_plugin = plugin;

    SetTitle(L"Unity Mesh Sync");
    SetOutSpace(0.4);

    const size_t buf_len = 128;
    wchar_t buf[buf_len];
    double outer_margin = 0.2;
    double inner_margin = 0.1;
    auto& s = getSettings();
    {
        MQFrame *vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        MQFrame *hf = CreateHorizontalFrame(vf);
        CreateLabel(hf, L"Server : Port");
        m_edit_server = CreateEdit(hf);
        m_edit_server->SetText(ToWCS(s.client_settings.server));
        m_edit_server->AddChangedEvent(this, &msmqSettingsDlg::OnServerChange);

        swprintf(buf, buf_len, L"%d", (int)s.client_settings.port);
        m_edit_port = CreateEdit(hf);
        m_edit_port->SetNumeric(MQEdit::NUMERIC_INT);
        m_edit_port->SetText(buf);
        m_edit_port->AddChangedEvent(this, &msmqSettingsDlg::OnPortChange);

        swprintf(buf, buf_len, L"%.3f", s.scale_factor);
        hf = CreateHorizontalFrame(vf);
        CreateLabel(hf, L"Scale Factor");
        m_edit_scale = CreateEdit(hf);
        m_edit_scale->SetNumeric(MQEdit::NUMERIC_DOUBLE);
        m_edit_scale->SetText(buf);
        m_edit_scale->AddChangedEvent(this, &msmqSettingsDlg::OnScaleChange);
    }

    {
        MQLabel *space = nullptr;
        MQFrame *vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

#if MQPLUGIN_VERSION >= 0x0460
        m_check_normals = CreateCheckBox(vf, L"Sync Normals");
        m_check_normals->SetChecked(s.sync_normals);
        m_check_normals->AddChangedEvent(this, &msmqSettingsDlg::OnSyncNormalsChange);
#endif

        m_check_vcolor = CreateCheckBox(vf, L"Sync Vertex Color");
        m_check_vcolor->SetChecked(s.sync_vertex_color);
        m_check_vcolor->AddChangedEvent(this, &msmqSettingsDlg::OnSyncVertexColorChange);

        m_check_double_sided = CreateCheckBox(vf, L"Double Sided");
        m_check_double_sided->SetChecked(s.make_double_sided);
        m_check_double_sided->AddChangedEvent(this, &msmqSettingsDlg::OnmakeDoubleSidedChange);

#if MQPLUGIN_VERSION >= 0x0464
        m_check_bones = CreateCheckBox(vf, L"Sync Bones");
        m_check_bones->SetChecked(s.sync_bones);
        m_check_bones->AddChangedEvent(this, &msmqSettingsDlg::OnSyncBonesChange);

        m_frame_poses = CreateHorizontalFrame(vf);
        space = CreateLabel(m_frame_poses, L" ");
        space->SetWidth(32);
        m_check_poses = CreateCheckBox(m_frame_poses, L"Sync Poses");
        m_check_poses->SetChecked(s.sync_poses);
        m_check_poses->AddChangedEvent(this, &msmqSettingsDlg::OnSyncPosesChange);
#endif

#if MQPLUGIN_VERSION >= 0x0470
        m_check_morphs = CreateCheckBox(vf, L"Sync Morphs");
        m_check_morphs->SetChecked(s.sync_morphs);
        m_check_morphs->AddChangedEvent(this, &msmqSettingsDlg::OnSyncMorphsChange);
#endif

        m_check_textures = CreateCheckBox(vf, L"Sync Textures");
        m_check_textures->SetChecked(s.sync_textures);
        m_check_textures->AddChangedEvent(this, &msmqSettingsDlg::OnSyncTexturesChange);

        m_check_camera = CreateCheckBox(vf, L"Sync Camera");
        m_check_camera->SetChecked(s.sync_camera);
        m_check_camera->AddChangedEvent(this, &msmqSettingsDlg::OnSyncCameraChange);

        m_frame_camera_path = CreateHorizontalFrame(vf);
        space = CreateLabel(m_frame_camera_path, L" ");
        space->SetWidth(32);
        CreateLabel(m_frame_camera_path, L"Camera Path");
        m_edit_camera_path = CreateEdit(m_frame_camera_path);
        m_edit_camera_path->SetText(ToWCS(s.host_camera_path));
        m_edit_camera_path->AddChangedEvent(this, &msmqSettingsDlg::OnCameraPathChange);
        m_edit_camera_path->SetHorzLayout(LAYOUT_FILL);
        m_frame_camera_path->SetVisible(m_check_camera->GetChecked());
    }

    {
        MQFrame *vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        m_check_autosync = CreateCheckBox(vf, L"Auto Sync");
        m_check_autosync->SetChecked(s.auto_sync);
        m_check_autosync->AddChangedEvent(this, &msmqSettingsDlg::OnAutoSyncChange);

        m_button_sync = CreateButton(vf, L"Manual Sync");
        m_button_sync->AddClickEvent(this, &msmqSettingsDlg::OnSyncClicked);
    }

    {
        MQFrame *vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        MQLabel *lb_import = CreateLabel(vf, L"Import Settings:");
        lb_import->SetFontBold(true);

        m_check_bake_skin = CreateCheckBox(vf, L"Bake Skin");
        m_check_bake_skin->SetChecked(s.bake_skin);
        m_check_bake_skin->AddChangedEvent(this, &msmqSettingsDlg::OnBakeSkinChange);

        m_check_bake_cloth = CreateCheckBox(vf, L"Bake Cloth");
        m_check_bake_cloth->SetChecked(s.bake_cloth);
        m_check_bake_cloth->AddChangedEvent(this, &msmqSettingsDlg::OnBakeClothChange);

        m_button_import = CreateButton(vf, L"Import Unity Scene");
        m_button_import->AddClickEvent(this, &msmqSettingsDlg::OnImportClicked);
    }

    {
        MQFrame *vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        std::string plugin_version = "Plugin Version: " msPluginVersionStr;
        CreateLabel(vf, ToWCS(plugin_version));
    }

    {
        MQFrame *vf = CreateVerticalFrame(this);
        vf->SetOutSpace(outer_margin);
        vf->SetInSpace(inner_margin);

        m_log = CreateLabel(vf, ToWCS(""));
    }

    this->AddHideEvent(this, &msmqSettingsDlg::OnHide);
}

msmqSettings& msmqSettingsDlg::getSettings()
{
    return m_plugin->getContext().getSettings();
}


BOOL msmqSettingsDlg::OnHide(MQWidgetBase *sender, MQDocument doc)
{
    m_plugin->WindowClose();
    return FALSE;
}

BOOL msmqSettingsDlg::OnServerChange(MQWidgetBase * sender, MQDocument doc)
{
    auto v = ToMBS(m_edit_server->GetText());
    getSettings().client_settings.server = v;
    return 0;
}

BOOL msmqSettingsDlg::OnPortChange(MQWidgetBase * sender, MQDocument doc)
{
    auto v = ToMBS(m_edit_port->GetText());
    getSettings().client_settings.server = std::atoi(v.c_str());
    return 0;
}

BOOL msmqSettingsDlg::OnScaleChange(MQWidgetBase * sender, MQDocument doc)
{
    auto v = ToMBS(m_edit_scale->GetText());
    auto f = std::atof(v.c_str());
    if (f != 0.0) {
        getSettings().scale_factor = (float)f;
        m_plugin->AutoSyncMeshes();
    }
    return 0;
}

BOOL msmqSettingsDlg::OnSyncNormalsChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().sync_normals = m_check_normals->GetChecked();
    m_plugin->AutoSyncMeshes();
    return 0;
}

BOOL msmqSettingsDlg::OnSyncVertexColorChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().sync_vertex_color = m_check_vcolor->GetChecked();
    m_plugin->AutoSyncMeshes();
    return 0;
}

BOOL msmqSettingsDlg::OnmakeDoubleSidedChange(MQWidgetBase * sender, MQDocument doc)
{
    getSettings().make_double_sided = m_check_double_sided->GetChecked();
    m_plugin->AutoSyncMeshes();
    return 0;
}

BOOL msmqSettingsDlg::OnSyncBonesChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().sync_bones = m_check_bones->GetChecked();
    m_frame_poses->SetVisible(m_check_bones->GetChecked());
    m_plugin->AutoSyncMeshes();
    return 0;
}

BOOL msmqSettingsDlg::OnSyncPosesChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().sync_poses = m_check_poses->GetChecked();
    m_plugin->AutoSyncMeshes();
    return 0;
}

BOOL msmqSettingsDlg::OnSyncMorphsChange(MQWidgetBase * sender, MQDocument doc)
{
    getSettings().sync_morphs = m_check_morphs->GetChecked();
    m_plugin->AutoSyncMeshes();
    return 0;
}

BOOL msmqSettingsDlg::OnSyncTexturesChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().sync_textures = m_check_textures->GetChecked();
    m_plugin->AutoSyncMeshes();
    return 0;
}

BOOL msmqSettingsDlg::OnSyncCameraChange(MQWidgetBase * sender, MQDocument doc)
{
    bool checked = m_check_camera->GetChecked();
    getSettings().sync_camera = checked;
    m_frame_camera_path->SetVisible(checked);
    m_plugin->AutoSyncCamera();
    return 0;
}

BOOL msmqSettingsDlg::OnCameraPathChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().host_camera_path = ToMBS(m_edit_camera_path->GetText());
    return 0;
}

BOOL msmqSettingsDlg::OnAutoSyncChange(MQWidgetBase *sender, MQDocument doc)
{
    if (m_check_autosync->GetChecked()) {
        auto& ctx = m_plugin->getContext();
        if (ctx.isServerAvailable()) {
            getSettings().auto_sync = true;
            m_plugin->Export();
        }
        else {
            m_check_autosync->SetChecked(false);
            LogInfo(ctx.getErrorMessage().c_str());
        }
    }
    else {
        getSettings().auto_sync = false;
    }
    return 0;
}

BOOL msmqSettingsDlg::OnSyncClicked(MQWidgetBase * sender, MQDocument doc)
{
    m_plugin->Export();
    return 0;
}


BOOL msmqSettingsDlg::OnBakeSkinChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().bake_skin = m_check_bake_skin->GetChecked();
    return 0;
}
BOOL msmqSettingsDlg::OnBakeClothChange(MQWidgetBase *sender, MQDocument doc)
{
    getSettings().bake_cloth = m_check_bake_cloth->GetChecked();
    return 0;
}

BOOL msmqSettingsDlg::OnImportClicked(MQWidgetBase * sender, MQDocument doc)
{
    m_plugin->Import();
    return 0;
}

void msmqSettingsDlg::LogInfo(const char *message)
{
    m_log->SetText(ToWCS(message));
}
