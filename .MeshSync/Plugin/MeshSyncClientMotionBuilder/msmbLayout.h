#pragma once


class msmbLayout : public FBDeviceLayout
{
    FBDeviceLayoutDeclare(msmbLayout, FBDeviceLayout);
public:
    bool FBCreate() override;
    void FBDestroy() override;

private:
    void onServerSettingsChange(HIRegister pCaller, HKEventBase pEvent);
    void onSceneSettingsChange(HIRegister pCaller, HKEventBase pEvent);
    void onAnimationSettingsChange(HIRegister pCaller, HKEventBase pEvent);
    void onAutoSync(HIRegister pCaller, HKEventBase pEvent);
    void onManualSync(HIRegister pCaller, HKEventBase pEvent);
    void onSyncAnimation(HIRegister pCaller, HKEventBase pEvent);

private:
    msmbDevice* m_device;

    FBLabel m_lb_server;
    FBLabel m_lb_address;
    FBEdit m_ed_address;
    FBLabel m_lb_port;
    FBEditNumber m_ed_port;

    FBLabel m_lb_scene;
    FBLabel m_lb_scale;
    FBEditNumber m_ed_scale;
    FBButton m_bu_auto_sync;
    FBButton m_bu_manual_sync;
    FBButton m_bu_sync_cameras;
    FBButton m_bu_sync_lights;
    FBButton m_bu_sync_meshes;
    FBButton m_bu_make_double_sided;
    FBButton m_bu_bake_deformers;

    FBLabel m_lb_animation;
    FBLabel m_lb_time_scale;
    FBEditNumber m_ed_time_scale;
    FBLabel m_lb_sps;
    FBEditNumber m_ed_sps;
    FBButton m_bu_kf_reduction;
    FBButton m_bu_keep_flat_curves;
    FBButton m_bu_sync_animations;

    FBLabel m_lb_version;
};
