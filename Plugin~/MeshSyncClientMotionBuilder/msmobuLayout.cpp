#include "pch.h"
#include "msmobuDevice.h"
#include "msmobuLayout.h"
#include "msmobuUtils.h"


FBDeviceLayoutImplementation(msmobuLayout);
FBRegisterDeviceLayout(msmobuLayout, "msmbDevice", FB_DEFAULT_SDK_ICON);

bool msmobuLayout::FBCreate()
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
    const char *idLabelFrameStep = "idLabelFrameStep";
    const char *idEditFrameStep = "idEditFrameStep";
    const char *idButtonSyncAnimations = "idButtonSyncAnimations";

    const char *idLabelVersion = "idLabelVersion";


    m_device = (msmobuDevice*)(FBDevice*)Device;
    auto& settings = m_device->getSettings();

    // server settings
    {
        AddRegion(idLabelServer, idLabelServer,
            lS, kFBAttachLeft, "", 1,
            lS, kFBAttachTop, "", 1,
            lW, kFBAttachNone, nullptr, 1,
            lH, kFBAttachNone, nullptr, 1);
        SetControl(idLabelServer, m_lb_server);
        m_lb_server.Caption = "Server";
        m_lb_server.Style = kFBTextStyleBold;

        AddRegion(idLabelAddress, idLabelAddress,
            0, kFBAttachLeft, idLabelServer, 1,
            lS, kFBAttachBottom, idLabelServer, 1,
            0, kFBAttachWidth, idLabelServer, 1,
            0, kFBAttachHeight, idLabelServer, 0.9f);
        SetControl(idLabelAddress, m_lb_address);
        m_lb_address.Caption = "Address ";

        AddRegion(idEditAddress, idEditAddress,
            lS, kFBAttachRight, idLabelAddress, 1,
            0, kFBAttachTop, idLabelAddress, 1,
            lW, kFBAttachNone, nullptr, 1,
            0, kFBAttachHeight, idLabelAddress, 1);
        SetControl(idEditAddress, m_ed_address);
        m_ed_address.Text = settings.client_settings.server.c_str();
        m_ed_address.OnChange.Add(this, (FBCallback)&msmobuLayout::onServerSettingsChange);

        AddRegion(idLabelPort, idLabelPort,
            0, kFBAttachLeft, idLabelAddress, 1,
            lS, kFBAttachBottom, idLabelAddress, 1,
            0, kFBAttachWidth, idLabelAddress, 1,
            0, kFBAttachHeight, idLabelAddress, 1);
        SetControl(idLabelPort, m_lb_port);
        m_lb_port.Caption = "Port ";

        AddRegion(idEditPort, idEditPort,
            lS, kFBAttachRight, idLabelPort, 1,
            0, kFBAttachTop, idLabelPort, 1,
            lW, kFBAttachNone, nullptr, 1,
            0, kFBAttachHeight, idLabelPort, 1);
        SetControl(idEditPort, m_ed_port);
        m_ed_port.Value = 8080;
        m_ed_port.Min = 0;
        m_ed_port.Max = 65535;
        m_ed_port.Precision = 1;
        m_ed_port.OnChange.Add(this, (FBCallback)&msmobuLayout::onServerSettingsChange);
    }

    // scene settings
    {
        AddRegion(idLabelScene, idLabelScene,
            0, kFBAttachLeft, idLabelPort, 1,
            lS2, kFBAttachBottom, idLabelPort, 1,
            0, kFBAttachWidth, idLabelPort, 1,
            0, kFBAttachHeight, idLabelPort, 1);
        SetControl(idLabelScene, m_lb_scene);
        m_lb_scene.Caption = "Scene";
        m_lb_scene.Style = kFBTextStyleBold;

        AddRegion(idLabelScale, idLabelScale,
            0, kFBAttachLeft, idLabelScene, 1,
            lS, kFBAttachBottom, idLabelScene, 1,
            0, kFBAttachWidth, idLabelScene, 1,
            0, kFBAttachHeight, idLabelScene, 1);
        SetControl(idLabelScale, m_lb_scale);
        m_lb_scale.Caption = "Scale Factor";

        AddRegion(idEditlScale, idEditlScale,
            lS, kFBAttachRight, idLabelScale, 1,
            0, kFBAttachTop, idLabelScale, 1,
            lW, kFBAttachNone, nullptr, 1,
            0, kFBAttachHeight, idLabelScale, 1);
        SetControl(idEditlScale, m_ed_scale);
        m_ed_scale.Value = (double)settings.scale_factor;
        m_ed_scale.OnChange.Add(this, (FBCallback)&msmobuLayout::onSceneSettingsChange);


        AddRegion(idButtonSyncMeshes, idButtonSyncMeshes,
            0, kFBAttachLeft, idLabelScale, 1,
            lS, kFBAttachBottom, idLabelScale, 1,
            lW2, kFBAttachWidth, idLabelScale, 1,
            0, kFBAttachHeight, idLabelScale, 1);
        SetControl(idButtonSyncMeshes, m_bu_sync_meshes);
        m_bu_sync_meshes.Caption = "Sync Meshes";
        m_bu_sync_meshes.Style = kFBCheckbox;
        m_bu_sync_meshes.State = (int)settings.sync_meshes;
        m_bu_sync_meshes.OnClick.Add(this, (FBCallback)&msmobuLayout::onSceneSettingsChange);
        m_bu_sync_meshes.Border.Spacing = 0;

        AddRegion(idButtonBothSided, idButtonBothSided,
            0, kFBAttachLeft, idButtonSyncMeshes, 1,
            lS, kFBAttachBottom, idButtonSyncMeshes, 1,
            0, kFBAttachWidth, idButtonSyncMeshes, 1,
            0, kFBAttachHeight, idButtonSyncMeshes, 1);
        SetControl(idButtonBothSided, m_bu_make_double_sided);
        m_bu_make_double_sided.Caption = "Make Double Sided";
        m_bu_make_double_sided.Style = kFBCheckbox;
        m_bu_make_double_sided.State = (int)settings.make_double_sided;
        m_bu_make_double_sided.OnClick.Add(this, (FBCallback)&msmobuLayout::onSceneSettingsChange);
        m_bu_make_double_sided.Border.Spacing = 0;

        AddRegion(idButtonBakeDeformers, idButtonBakeDeformers,
            0, kFBAttachLeft, idButtonBothSided, 1,
            lS, kFBAttachBottom, idButtonBothSided, 1,
            0, kFBAttachWidth, idButtonBothSided, 1,
            0, kFBAttachHeight, idButtonBothSided, 1);
        SetControl(idButtonBakeDeformers, m_bu_bake_deformers);
        m_bu_bake_deformers.Caption = "Bake Deformers";
        m_bu_bake_deformers.Style = kFBCheckbox;
        m_bu_bake_deformers.State = (int)settings.bake_deformars;
        m_bu_bake_deformers.OnClick.Add(this, (FBCallback)&msmobuLayout::onSceneSettingsChange);
        m_bu_bake_deformers.Border.Spacing = 0;

        AddRegion(idButtonSyncCameras, idButtonSyncCameras,
            0, kFBAttachLeft, idButtonBakeDeformers, 1,
            lS, kFBAttachBottom, idButtonBakeDeformers, 1,
            0, kFBAttachWidth, idButtonBakeDeformers, 1,
            0, kFBAttachHeight, idButtonBakeDeformers, 1);
        SetControl(idButtonSyncCameras, m_bu_sync_cameras);
        m_bu_sync_cameras.Caption = "Sync Cameras";
        m_bu_sync_cameras.Style = kFBCheckbox;
        m_bu_sync_cameras.State = (int)settings.sync_cameras;
        m_bu_sync_cameras.OnClick.Add(this, (FBCallback)&msmobuLayout::onSceneSettingsChange);

        AddRegion(idButtonSyncLights, idButtonSyncLights,
            0, kFBAttachLeft, idButtonSyncCameras, 1,
            lS, kFBAttachBottom, idButtonSyncCameras, 1,
            0, kFBAttachWidth, idButtonSyncCameras, 1,
            0, kFBAttachHeight, idButtonSyncCameras, 1);
        SetControl(idButtonSyncLights, m_bu_sync_lights);
        m_bu_sync_lights.Caption = "Sync Lights";
        m_bu_sync_lights.Style = kFBCheckbox;
        m_bu_sync_lights.State = (int)settings.sync_lights;
        m_bu_sync_lights.OnClick.Add(this, (FBCallback)&msmobuLayout::onSceneSettingsChange);


        AddRegion(idButtonAutoSync, idButtonAutoSync,
            0, kFBAttachLeft, idButtonSyncLights, 1,
            lS2, kFBAttachBottom, idButtonSyncLights, 1,
            0, kFBAttachWidth, idButtonSyncLights, 1,
            0, kFBAttachHeight, idButtonSyncLights, 1);
        SetControl(idButtonAutoSync, m_bu_auto_sync);
        m_bu_auto_sync.Caption = "Auto Sync";
        m_bu_auto_sync.Style = kFBCheckbox;
        m_bu_auto_sync.State = (int)settings.auto_sync;
        m_bu_auto_sync.OnClick.Add(this, (FBCallback)&msmobuLayout::onAutoSync);

        AddRegion(idButtonManualSync, idButtonManualSync,
            0, kFBAttachLeft, idButtonAutoSync, 1,
            lS, kFBAttachBottom, idButtonAutoSync, 1,
            lW2, kFBAttachNone, nullptr, 1,
            0, kFBAttachHeight, idButtonAutoSync, 1);
        SetControl(idButtonManualSync, m_bu_manual_sync);
        m_bu_manual_sync.Caption = "Manual Sync";
        m_bu_manual_sync.OnClick.Add(this, (FBCallback)&msmobuLayout::onManualSync);
    }

    // animation settings
    {
        AddRegion(idLabelAnimation, idLabelAnimation,
            0, kFBAttachLeft, idButtonManualSync, 1,
            lS2, kFBAttachBottom, idButtonManualSync, 1,
            lW2, kFBAttachNone, nullptr, 1,
            0, kFBAttachHeight, idButtonManualSync, 1);
        SetControl(idLabelAnimation, m_lb_animation);
        m_lb_animation.Caption = "Animation";
        m_lb_animation.Style = kFBTextStyleBold;

        AddRegion(idLabelFrameStep, idLabelFrameStep,
            0, kFBAttachLeft, idLabelAnimation, 1,
            lS, kFBAttachBottom, idLabelAnimation, 1,
            lW, kFBAttachNone, nullptr, 1,
            0, kFBAttachHeight, idLabelAnimation, 1);
        SetControl(idLabelFrameStep, m_lb_frame_step);
        m_lb_frame_step.Caption = "Frame Step";

        AddRegion(idEditFrameStep, idEditFrameStep,
            lS, kFBAttachRight, idLabelFrameStep, 1,
            0, kFBAttachTop, idLabelFrameStep, 1,
            lW, kFBAttachNone, nullptr, 1,
            0, kFBAttachHeight, idLabelFrameStep, 1);
        SetControl(idEditFrameStep, m_ed_frame_step);
        m_ed_frame_step.Value = 1;
        m_ed_frame_step.Min = 0.0;
        m_ed_frame_step.SmallStep = 1.0;
        m_ed_frame_step.LargeStep = 1.0;
        m_ed_frame_step.OnChange.Add(this, (FBCallback)&msmobuLayout::onAnimationSettingsChange);

        AddRegion(idButtonSyncAnimations, idButtonSyncAnimations,
            0, kFBAttachLeft, idLabelFrameStep, 1,
            lS, kFBAttachBottom, idLabelFrameStep, 1,
            lW2, kFBAttachNone, nullptr, 1,
            0, kFBAttachHeight, idLabelFrameStep, 1);
        SetControl(idButtonSyncAnimations, m_bu_sync_animations);
        m_bu_sync_animations.Caption = "Sync Animations";
        m_bu_sync_animations.OnClick.Add(this, (FBCallback)&msmobuLayout::onSyncAnimation);
    }

    {
        AddRegion(idLabelVersion, idLabelVersion,
            0, kFBAttachLeft, idButtonSyncAnimations, 1,
            lS2, kFBAttachBottom, idButtonSyncAnimations, 1,
            lW2, kFBAttachNone, nullptr, 1,
            0, kFBAttachHeight, idButtonSyncAnimations, 1);
        SetControl(idLabelVersion, m_lb_version);
        m_lb_version.Caption = "Plugin Version: " msPluginVersionStr;
    }

    return true;
}

void msmobuLayout::FBDestroy()
{
}


void msmobuLayout::onServerSettingsChange(HIRegister pCaller, HKEventBase pEvent)
{
    auto& settings = m_device->getSettings();
    settings.client_settings.server = m_ed_address.Text;
    settings.client_settings.port = (uint16_t)m_ed_port.Value;
}

void msmobuLayout::onSceneSettingsChange(HIRegister pCaller, HKEventBase pEvent)
{
    auto& settings = m_device->getSettings();
    settings.scale_factor = (float)m_ed_scale.Value;
    settings.sync_meshes = (bool)(int)m_bu_sync_meshes.State;
    settings.make_double_sided = (bool)(int)m_bu_make_double_sided.State;
    settings.bake_deformars = (bool)(int)m_bu_bake_deformers.State;
    settings.sync_cameras = (bool)(int)m_bu_sync_cameras.State;
    settings.sync_lights = (bool)(int)m_bu_sync_lights.State;
    if (settings.auto_sync)
        m_device->sendObjects(true);
}

void msmobuLayout::onAnimationSettingsChange(HIRegister pCaller, HKEventBase pEvent)
{
    auto& settings = m_device->getSettings();
    settings.frame_step = (float)m_ed_frame_step.Value;
}

void msmobuLayout::onAutoSync(HIRegister pCaller, HKEventBase pEvent)
{
    auto& ctx = *m_device;
    auto& settings = m_device->getSettings();
    if ((bool)(int)m_bu_auto_sync.State) {
        if (ctx.isServerAvailable()) {
            settings.auto_sync = true;
            m_device->sendObjects(false);
        }
        else {
            ctx.logError("MeshSync: Server not available. %s\n", ctx.getErrorMessage().c_str());
            m_bu_auto_sync.State = 0;
        }
    }
    else {
        settings.auto_sync = false;
    }
}

void msmobuLayout::onManualSync(HIRegister pCaller, HKEventBase pEvent)
{
    auto& ctx = *m_device;
    if (!ctx.isServerAvailable()) {
        ctx.logError("MeshSync: Server not available. %s\n", ctx.getErrorMessage().c_str());
        return;
    }
    ctx.wait();
    ctx.sendObjects(true);
}

void msmobuLayout::onSyncAnimation(HIRegister pCaller, HKEventBase pEvent)
{
    auto& ctx = *m_device;
    if (!ctx.isServerAvailable()) {
        ctx.logError("MeshSync: Server not available. %s\n", ctx.getErrorMessage().c_str());
        return;
    }
    ctx.wait();
    ctx.sendAnimations();
}
