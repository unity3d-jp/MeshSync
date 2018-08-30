#include "pch.h"
#include "msmbDevice.h"
#include "msmbLayout.h"
#include "msmbUtils.h"


FBDeviceLayoutImplementation(msmbLayout);
FBRegisterDeviceLayout(msmbLayout, "msmbDevice", FB_DEFAULT_SDK_ICON);

bool msmbLayout::FBCreate()
{
    m_device = (msmbDevice*)(FBDevice*)Device;

    const int lS = 5;
    const int lS2 = 10;
    const int lW = 100;
    const int lW2 = 150;
    const int lH = 18;
    const int lH2 = 200;

    // server settings
    {
        AddRegion("LabelServer", "LabelServer",
            lS, kFBAttachLeft, "", 1.0,
            lS, kFBAttachTop, "", 1.0,
            lW, kFBAttachNone, NULL, 1.0,
            lH, kFBAttachNone, NULL, 1.0);
        SetControl("LabelServer", m_lb_server);
        m_lb_server.Caption = "Server";

        AddRegion("LabelAddress", "LabelAddress",
            0, kFBAttachLeft, "LabelServer", 1.0,
            lS, kFBAttachBottom, "LabelServer", 1.0,
            0, kFBAttachWidth, "LabelServer", 1.0,
            0, kFBAttachHeight, "LabelServer", 1.0);
        SetControl("LabelAddress", m_lb_address);
        m_lb_address.Caption = "Address ";

        AddRegion("EditAddress", "EditAddress",
            lS, kFBAttachRight, "LabelAddress", 1.0,
            0, kFBAttachTop, "LabelAddress", 1.0,
            lW, kFBAttachNone, NULL, 1.0,
            0, kFBAttachHeight, "LabelAddress", 1.0);
        SetControl("EditAddress", m_ed_address);
        m_ed_address.Text = m_device->client_settings.server.c_str();

        AddRegion("LabelPort", "LabelPort",
            0, kFBAttachLeft, "LabelAddress", 1.0,
            lS, kFBAttachBottom, "LabelAddress", 1.0,
            0, kFBAttachWidth, "LabelAddress", 1.0,
            0, kFBAttachHeight, "LabelAddress", 1.0);
        SetControl("LabelPort", m_lb_port);
        m_lb_port.Caption = "Port ";

        AddRegion("EditPort", "EditPort",
            lS, kFBAttachRight, "LabelPort", 1.0,
            0, kFBAttachTop, "LabelPort", 1.0,
            lW, kFBAttachNone, NULL, 1.0,
            0, kFBAttachHeight, "LabelPort", 1.0);
        SetControl("EditPort", m_ed_port);
        m_ed_port.Text = "8080";
    }

    // scene settings
    {
        AddRegion("LabelScene", "LabelScene",
            0, kFBAttachLeft, "LabelPort", 1.0,
            lS2, kFBAttachBottom, "LabelPort", 1.0,
            0, kFBAttachWidth, "LabelPort", 1.0,
            0, kFBAttachHeight, "LabelPort", 1.0);
        SetControl("LabelScene", m_lb_scene);
        m_lb_scene.Caption = "Scene";

        AddRegion("LabelScale", "LabelScale",
            0, kFBAttachLeft, "LabelScene", 1.0,
            lS, kFBAttachBottom, "LabelScene", 1.0,
            0, kFBAttachWidth, "LabelScene", 1.0,
            0, kFBAttachHeight, "LabelScene", 1.0);
        SetControl("LabelScale", m_lb_scale);
        m_lb_scale.Caption = "Scale Factor";

        AddRegion("EditScale", "EditScale",
            lS, kFBAttachRight, "LabelScale", 1.0,
            0, kFBAttachTop, "LabelScale", 1.0,
            lW, kFBAttachNone, NULL, 1.0,
            0, kFBAttachHeight, "LabelScale", 1.0);
        SetControl("EditScale", m_ed_scale);
        m_ed_scale.Value = 1.0;


        AddRegion("ButtonSyncMeshes", "ButtonSyncMeshes",
            0, kFBAttachLeft, "LabelScale", 1.0,
            lS, kFBAttachBottom, "LabelScale", 1.0,
            0, kFBAttachWidth, "LabelScale", 1.0,
            0, kFBAttachHeight, "LabelScale", 1.0);
        SetControl("ButtonSyncMeshes", m_bu_sync_meshes);
        m_bu_sync_meshes.Caption = "Sync Meshes";
        m_bu_sync_meshes.Style = kFBCheckbox;

        AddRegion("ButtonSyncCameras", "ButtonSyncCameras",
            0, kFBAttachLeft, "ButtonSyncMeshes", 1.0,
            lS, kFBAttachBottom, "ButtonSyncMeshes", 1.0,
            0, kFBAttachWidth, "ButtonSyncMeshes", 1.0,
            0, kFBAttachHeight, "ButtonSyncMeshes", 1.0);
        SetControl("ButtonSyncCameras", m_bu_sync_cameras);
        m_bu_sync_cameras.Caption = "Sync Cameras";
        m_bu_sync_cameras.Style = kFBCheckbox;

        AddRegion("ButtonSyncLights", "ButtonSyncLights",
            0, kFBAttachLeft, "ButtonSyncCameras", 1.0,
            lS, kFBAttachBottom, "ButtonSyncCameras", 1.0,
            0, kFBAttachWidth, "ButtonSyncCameras", 1.0,
            0, kFBAttachHeight, "ButtonSyncCameras", 1.0);
        SetControl("ButtonSyncLights", m_bu_sync_lights);
        m_bu_sync_lights.Caption = "Sync Lights";
        m_bu_sync_lights.Style = kFBCheckbox;


        AddRegion("ButtonAutoSync", "ButtonAutoSync",
            0, kFBAttachLeft, "ButtonSyncLights", 1.0,
            lS2, kFBAttachBottom, "ButtonSyncLights", 1.0,
            0, kFBAttachWidth, "ButtonSyncLights", 1.0,
            0, kFBAttachHeight, "ButtonSyncLights", 1.0);
        SetControl("ButtonAutoSync", m_bu_auto_sync);
        m_bu_auto_sync.Caption = "Auto Sync";
        m_bu_auto_sync.Style = kFBCheckbox;

        AddRegion("ButtonManualSync", "ButtonManualSync",
            0, kFBAttachLeft, "ButtonAutoSync", 1.0,
            lS, kFBAttachBottom, "ButtonAutoSync", 1.0,
            0, kFBAttachWidth, "ButtonAutoSync", 1.0,
            0, kFBAttachHeight, "ButtonAutoSync", 1.0);
        SetControl("ButtonManualSync", m_bu_manual_sync);
        m_bu_manual_sync.Caption = "Manual Sync";
    }

    // animation settings
    {
        AddRegion("LabelAnimation", "LabelAnimation",
            0, kFBAttachLeft, "ButtonManualSync", 1.0,
            lS2, kFBAttachBottom, "ButtonManualSync", 1.0,
            0, kFBAttachWidth, "ButtonManualSync", 1.0,
            0, kFBAttachHeight, "ButtonManualSync", 1.0);
        SetControl("LabelAnimation", m_lb_animation);
        m_lb_animation.Caption = "Animation";


        AddRegion("LabelTimeScale", "LabelTimeScale",
            0, kFBAttachLeft, "LabelAnimation", 1.0,
            lS, kFBAttachBottom, "LabelAnimation", 1.0,
            0, kFBAttachWidth, "LabelAnimation", 1.0,
            0, kFBAttachHeight, "LabelAnimation", 1.0);
        SetControl("LabelTimeScale", m_lb_time_scale);
        m_lb_time_scale.Caption = "Time Scale";

        AddRegion("EditTimeScale", "EditTimeScale",
            lS, kFBAttachRight, "LabelTimeScale", 1.0,
            0, kFBAttachTop, "LabelTimeScale", 1.0,
            lW, kFBAttachNone, NULL, 1.0,
            0, kFBAttachHeight, "LabelTimeScale", 1.0);
        SetControl("EditTimeScale", m_ed_time_scale);
        m_ed_time_scale.Value = 1.0;


        AddRegion("LabelSPS", "LabelSPS",
            0, kFBAttachLeft, "LabelTimeScale", 1.0,
            lS, kFBAttachBottom, "LabelTimeScale", 1.0,
            0, kFBAttachWidth, "LabelTimeScale", 1.0,
            0, kFBAttachHeight, "LabelTimeScale", 1.0);
        SetControl("LabelSPS", m_lb_sps);
        m_lb_sps.Caption = "Samples Per Second";

        AddRegion("EditSPS", "EditSPS",
            lS, kFBAttachRight, "LabelSPS", 1.0,
            0, kFBAttachTop, "LabelSPS", 1.0,
            lW, kFBAttachNone, NULL, 1.0,
            0, kFBAttachHeight, "LabelSPS", 1.0);
        SetControl("EditSPS", m_ed_sps);
        m_ed_sps.Value = 3.0;


        AddRegion("ButtonSyncAnimations", "ButtonSyncAnimations",
            0, kFBAttachLeft, "LabelSPS", 1.0,
            lS2, kFBAttachBottom, "LabelSPS", 1.0,
            0, kFBAttachWidth, "LabelSPS", 1.0,
            0, kFBAttachHeight, "LabelSPS", 1.0);
        SetControl("ButtonSyncAnimations", m_bu_sync_animations);
        m_bu_sync_animations.Caption = "Sync Animations";
    }

    return true;
}

void msmbLayout::FBDestroy()
{
}
