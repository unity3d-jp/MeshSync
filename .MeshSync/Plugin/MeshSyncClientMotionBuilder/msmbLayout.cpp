#include "pch.h"
#include "msmbDevice.h"
#include "msmbLayout.h"
#include "msmbUtils.h"


FBDeviceLayoutImplementation(msmbLayout);
FBRegisterDeviceLayout(msmbLayout, "msmbDevice", FB_DEFAULT_SDK_ICON);

bool msmbLayout::FBCreate()
{
    const int lS = 5;
    const int lS2 = 10;
    const int lW = 170;
    const int lW2 = 340;
    const int lH = 18;

    const char *idLabelServer = "idLabelServer";
    const char *idLabelAddress = "idLabelAddress";
    const char *idEditAddress = "idEditAddress";
    const char *idLabelPort = "idLabelPort";
    const char *idEditPort = "idEditPort";

    const char *idLabelScene = "idLabelScene";
    const char *idLabelScale = "idLabelScale";
    const char *idEditlScale = "idEditlScale";
    const char *idButtonSyncMeshes = "idButtonSyncMeshes";
    const char *idButtonBothSided = "idButtonBothSided";
    const char *idButtonBakeDeformers = "idButtonBakeDeformers";
    const char *idButtonSyncCameras = "idButtonSyncCameras";
    const char *idButtonSyncLights = "idButtonSyncLights";
    const char *idButtonAutoSync = "idButtonAutoSync";
    const char *idButtonManualSync = "idButtonManualSync";

    const char *idLabelAnimation = "idLabelAnimation";
    const char *idLabelTimeScale = "idLabelTimeScale";
    const char *idEditTimeScale = "idEditTimeScale";
    const char *idLabelSPS = "idLabelSPS";
    const char *idEditSPS = "idEditSPS";
    const char *idButtonKFReduction = "idButtonKFReduction";
    const char *idButtonSyncAnimations = "idButtonSyncAnimations";

    const char *idLabelVersion = "idLabelVersion";


    m_device = (msmbDevice*)(FBDevice*)Device;

    // server settings
    {
        AddRegion(idLabelServer, idLabelServer,
            lS, kFBAttachLeft, "", 1.0,
            lS, kFBAttachTop, "", 1.0,
            lW, kFBAttachNone, nullptr, 1.0,
            lH, kFBAttachNone, nullptr, 1.0);
        SetControl(idLabelServer, m_lb_server);
        m_lb_server.Caption = "Server";
        m_lb_server.Style = kFBTextStyleBold;

        AddRegion(idLabelAddress, idLabelAddress,
            0, kFBAttachLeft, idLabelServer, 1.0,
            lS, kFBAttachBottom, idLabelServer, 1.0,
            0, kFBAttachWidth, idLabelServer, 1.0,
            0, kFBAttachHeight, idLabelServer, 1.0);
        SetControl(idLabelAddress, m_lb_address);
        m_lb_address.Caption = "Address ";

        AddRegion(idEditAddress, idEditAddress,
            lS, kFBAttachRight, idLabelAddress, 1.0,
            0, kFBAttachTop, idLabelAddress, 1.0,
            lW, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idLabelAddress, 1.0);
        SetControl(idEditAddress, m_ed_address);
        m_ed_address.Text = m_device->client_settings.server.c_str();
        m_ed_address.OnChange.Add(this, (FBCallback)&msmbLayout::onServerSettingsChange);

        AddRegion(idLabelPort, idLabelPort,
            0, kFBAttachLeft, idLabelAddress, 1.0,
            lS, kFBAttachBottom, idLabelAddress, 1.0,
            0, kFBAttachWidth, idLabelAddress, 1.0,
            0, kFBAttachHeight, idLabelAddress, 1.0);
        SetControl(idLabelPort, m_lb_port);
        m_lb_port.Caption = "Port ";

        AddRegion(idEditPort, idEditPort,
            lS, kFBAttachRight, idLabelPort, 1.0,
            0, kFBAttachTop, idLabelPort, 1.0,
            lW, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idLabelPort, 1.0);
        SetControl(idEditPort, m_ed_port);
        m_ed_port.Value = 8080;
        m_ed_port.Min = 0;
        m_ed_port.Max = 65535;
        m_ed_port.Precision = 1;
        m_ed_port.OnChange.Add(this, (FBCallback)&msmbLayout::onServerSettingsChange);
    }

    // scene settings
    {
        AddRegion(idLabelScene, idLabelScene,
            0, kFBAttachLeft, idLabelPort, 1.0,
            lS2, kFBAttachBottom, idLabelPort, 1.0,
            0, kFBAttachWidth, idLabelPort, 1.0,
            0, kFBAttachHeight, idLabelPort, 1.0);
        SetControl(idLabelScene, m_lb_scene);
        m_lb_scene.Caption = "Scene";
        m_lb_scene.Style = kFBTextStyleBold;

        AddRegion(idLabelScale, idLabelScale,
            0, kFBAttachLeft, idLabelScene, 1.0,
            lS, kFBAttachBottom, idLabelScene, 1.0,
            0, kFBAttachWidth, idLabelScene, 1.0,
            0, kFBAttachHeight, idLabelScene, 1.0);
        SetControl(idLabelScale, m_lb_scale);
        m_lb_scale.Caption = "Scale Factor";

        AddRegion(idEditlScale, idEditlScale,
            lS, kFBAttachRight, idLabelScale, 1.0,
            0, kFBAttachTop, idLabelScale, 1.0,
            lW, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idLabelScale, 1.0);
        SetControl(idEditlScale, m_ed_scale);
        m_ed_scale.Value = (double)m_device->scale_factor;
        m_ed_scale.OnChange.Add(this, (FBCallback)&msmbLayout::onSceneSettingsChange);


        AddRegion(idButtonSyncMeshes, idButtonSyncMeshes,
            0, kFBAttachLeft, idLabelScale, 1.0,
            lS, kFBAttachBottom, idLabelScale, 1.0,
            lW2, kFBAttachWidth, idLabelScale, 1.0,
            0, kFBAttachHeight, idLabelScale, 1.0);
        SetControl(idButtonSyncMeshes, m_bu_sync_meshes);
        m_bu_sync_meshes.Caption = "Sync Meshes";
        m_bu_sync_meshes.Style = kFBCheckbox;
        m_bu_sync_meshes.State = (int)m_device->sync_meshes;
        m_bu_sync_meshes.OnClick.Add(this, (FBCallback)&msmbLayout::onSceneSettingsChange);

        AddRegion(idButtonBothSided, idButtonBothSided,
            0, kFBAttachLeft, idButtonSyncMeshes, 1.0,
            lS, kFBAttachBottom, idButtonSyncMeshes, 1.0,
            0, kFBAttachWidth, idButtonSyncMeshes, 1.0,
            0, kFBAttachHeight, idButtonSyncMeshes, 1.0);
        SetControl(idButtonBothSided, m_bu_make_double_sided);
        m_bu_make_double_sided.Caption = "Double Sided";
        m_bu_make_double_sided.Style = kFBCheckbox;
        m_bu_make_double_sided.State = (int)m_device->make_double_sided;
        m_bu_make_double_sided.OnClick.Add(this, (FBCallback)&msmbLayout::onSceneSettingsChange);

        AddRegion(idButtonBakeDeformers, idButtonBakeDeformers,
            0, kFBAttachLeft, idButtonBothSided, 1.0,
            lS, kFBAttachBottom, idButtonBothSided, 1.0,
            0, kFBAttachWidth, idButtonBothSided, 1.0,
            0, kFBAttachHeight, idButtonBothSided, 1.0);
        SetControl(idButtonBakeDeformers, m_bu_bake_deformers);
        m_bu_bake_deformers.Caption = "Bake Deformers";
        m_bu_bake_deformers.Style = kFBCheckbox;
        m_bu_bake_deformers.State = (int)m_device->bake_deformars;
        m_bu_bake_deformers.OnClick.Add(this, (FBCallback)&msmbLayout::onSceneSettingsChange);

        AddRegion(idButtonSyncCameras, idButtonSyncCameras,
            0, kFBAttachLeft, idButtonBakeDeformers, 1.0,
            lS, kFBAttachBottom, idButtonBakeDeformers, 1.0,
            0, kFBAttachWidth, idButtonBakeDeformers, 1.0,
            0, kFBAttachHeight, idButtonBakeDeformers, 1.0);
        SetControl(idButtonSyncCameras, m_bu_sync_cameras);
        m_bu_sync_cameras.Caption = "Sync Cameras";
        m_bu_sync_cameras.Style = kFBCheckbox;
        m_bu_sync_cameras.State = (int)m_device->sync_cameras;
        m_bu_sync_cameras.OnClick.Add(this, (FBCallback)&msmbLayout::onSceneSettingsChange);

        AddRegion(idButtonSyncLights, idButtonSyncLights,
            0, kFBAttachLeft, idButtonSyncCameras, 1.0,
            lS, kFBAttachBottom, idButtonSyncCameras, 1.0,
            0, kFBAttachWidth, idButtonSyncCameras, 1.0,
            0, kFBAttachHeight, idButtonSyncCameras, 1.0);
        SetControl(idButtonSyncLights, m_bu_sync_lights);
        m_bu_sync_lights.Caption = "Sync Lights";
        m_bu_sync_lights.Style = kFBCheckbox;
        m_bu_sync_lights.State = (int)m_device->sync_lights;
        m_bu_sync_lights.OnClick.Add(this, (FBCallback)&msmbLayout::onSceneSettingsChange);


        AddRegion(idButtonAutoSync, idButtonAutoSync,
            0, kFBAttachLeft, idButtonSyncLights, 1.0,
            lS2, kFBAttachBottom, idButtonSyncLights, 1.0,
            0, kFBAttachWidth, idButtonSyncLights, 1.0,
            0, kFBAttachHeight, idButtonSyncLights, 1.0);
        SetControl(idButtonAutoSync, m_bu_auto_sync);
        m_bu_auto_sync.Caption = "Auto Sync";
        m_bu_auto_sync.Style = kFBCheckbox;
        m_bu_auto_sync.State = (int)m_device->auto_sync;
        m_bu_auto_sync.OnClick.Add(this, (FBCallback)&msmbLayout::onAutoSync);

        AddRegion(idButtonManualSync, idButtonManualSync,
            0, kFBAttachLeft, idButtonAutoSync, 1.0,
            lS, kFBAttachBottom, idButtonAutoSync, 1.0,
            lW2, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idButtonAutoSync, 1.0);
        SetControl(idButtonManualSync, m_bu_manual_sync);
        m_bu_manual_sync.Caption = "Manual Sync";
        m_bu_manual_sync.OnClick.Add(this, (FBCallback)&msmbLayout::onManualSync);
    }

    // animation settings
    {
        AddRegion(idLabelAnimation, idLabelAnimation,
            0, kFBAttachLeft, idButtonManualSync, 1.0,
            lS2, kFBAttachBottom, idButtonManualSync, 1.0,
            lW2, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idButtonManualSync, 1.0);
        SetControl(idLabelAnimation, m_lb_animation);
        m_lb_animation.Caption = "Animation";
        m_lb_animation.Style = kFBTextStyleBold;


        AddRegion(idLabelTimeScale, idLabelTimeScale,
            0, kFBAttachLeft, idLabelAnimation, 1.0,
            lS, kFBAttachBottom, idLabelAnimation, 1.0,
            lW, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idLabelAnimation, 1.0);
        SetControl(idLabelTimeScale, m_lb_time_scale);
        m_lb_time_scale.Caption = "Time Scale";

        AddRegion(idEditTimeScale, idEditTimeScale,
            lS, kFBAttachRight, idLabelTimeScale, 1.0,
            0, kFBAttachTop, idLabelTimeScale, 1.0,
            lW, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idLabelTimeScale, 1.0);
        SetControl(idEditTimeScale, m_ed_time_scale);
        m_ed_time_scale.Value = 1.0;
        m_ed_time_scale.Min = 0.01;
        m_ed_time_scale.SmallStep = 0.01;
        m_ed_time_scale.LargeStep = 0.1;
        m_ed_time_scale.OnChange.Add(this, (FBCallback)&msmbLayout::onAnimationSettingsChange);


        AddRegion(idLabelSPS, idLabelSPS,
            0, kFBAttachLeft, idLabelTimeScale, 1.0,
            lS, kFBAttachBottom, idLabelTimeScale, 1.0,
            lW, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idLabelTimeScale, 1.0);
        SetControl(idLabelSPS, m_lb_sps);
        m_lb_sps.Caption = "Samples Per Sec";

        AddRegion(idEditSPS, idEditSPS,
            lS, kFBAttachRight, idLabelSPS, 1.0,
            0, kFBAttachTop, idLabelSPS, 1.0,
            lW, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idLabelSPS, 1.0);
        SetControl(idEditSPS, m_ed_sps);
        m_ed_sps.Value = 3;
        m_ed_sps.Min = 0.01;
        m_ed_sps.SmallStep = 0.01;
        m_ed_sps.LargeStep = 0.1;
        m_ed_sps.OnChange.Add(this, (FBCallback)&msmbLayout::onAnimationSettingsChange);

        AddRegion(idButtonKFReduction, idButtonKFReduction,
            0, kFBAttachLeft, idLabelSPS, 1.0,
            lS, kFBAttachBottom, idLabelSPS, 1.0,
            0, kFBAttachWidth, idLabelSPS, 1.0,
            0, kFBAttachHeight, idLabelSPS, 1.0);
        SetControl(idButtonKFReduction, m_bu_kf_reduction);
        m_bu_kf_reduction.Caption = "Keyframe Reduction";
        m_bu_kf_reduction.Style = kFBCheckbox;
        m_bu_kf_reduction.State = (int)m_device->keyframe_reduction;
        m_bu_kf_reduction.OnClick.Add(this, (FBCallback)&msmbLayout::onAnimationSettingsChange);

        AddRegion(idButtonSyncAnimations, idButtonSyncAnimations,
            0, kFBAttachLeft, idButtonKFReduction, 1.0,
            lS, kFBAttachBottom, idButtonKFReduction, 1.0,
            lW2, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idButtonKFReduction, 1.0);
        SetControl(idButtonSyncAnimations, m_bu_sync_animations);
        m_bu_sync_animations.Caption = "Sync Animations";
        m_bu_sync_animations.OnClick.Add(this, (FBCallback)&msmbLayout::onSyncAnimation);
    }

    {
        AddRegion(idLabelVersion, idLabelVersion,
            0, kFBAttachLeft, idButtonSyncAnimations, 1.0,
            lS2, kFBAttachBottom, idButtonSyncAnimations, 1.0,
            lW2, kFBAttachNone, nullptr, 1.0,
            0, kFBAttachHeight, idButtonSyncAnimations, 1.0);
        SetControl(idLabelVersion, m_lb_version);
        m_lb_version.Caption = "Plugin Version: " msReleaseDateStr;
    }

    return true;
}

void msmbLayout::FBDestroy()
{
}


void msmbLayout::onServerSettingsChange(HIRegister pCaller, HKEventBase pEvent)
{
    m_device->client_settings.server = m_ed_address.Text;
    m_device->client_settings.port = (uint16_t)m_ed_port.Value;
}

void msmbLayout::onSceneSettingsChange(HIRegister pCaller, HKEventBase pEvent)
{
    m_device->scale_factor = (float)m_ed_scale.Value;
    m_device->sync_meshes = (bool)(int)m_bu_sync_meshes.State;
    m_device->make_double_sided = (bool)(int)m_bu_make_double_sided.State;
    m_device->bake_deformars = (bool)(int)m_bu_bake_deformers.State;
    m_device->sync_cameras = (bool)(int)m_bu_sync_cameras.State;
    m_device->sync_lights = (bool)(int)m_bu_sync_lights.State;
    if (m_device->auto_sync)
        m_device->sendScene(true);
}

void msmbLayout::onAnimationSettingsChange(HIRegister pCaller, HKEventBase pEvent)
{
    m_device->animation_time_scale = (float)m_ed_time_scale.Value;
    m_device->animation_sps = (float)m_ed_sps.Value;
    m_device->keyframe_reduction = (bool)(int)m_bu_kf_reduction.State;
}

void msmbLayout::onAutoSync(HIRegister pCaller, HKEventBase pEvent)
{
    m_device->auto_sync = (bool)(int)m_bu_auto_sync.State;
    if (m_device->auto_sync)
        m_device->sendScene(false);
}

void msmbLayout::onManualSync(HIRegister pCaller, HKEventBase pEvent)
{
    m_device->sendScene(true);
}

void msmbLayout::onSyncAnimation(HIRegister pCaller, HKEventBase pEvent)
{
    m_device->sendAnimations();
}
