#pragma once


class msmbLayout : public FBDeviceLayout
{
    FBDeviceLayoutDeclare(msmbLayout, FBDeviceLayout);
public:
    bool FBCreate() override;
    void FBDestroy() override;

private:
    FBSystem    m_system;
    msmbDevice* m_device;

    FBLabel m_lb_server;
    FBEdit m_ed_server_address;
    FBEditNumber m_ed_server_port;

    FBLabel m_lb_scene;
    FBEditNumber m_ed_scale_factor;
    FBButton m_bu_auto_sync;
    FBButton m_bu_manual_sync;
    FBButton m_bu_sync_cameras;
    FBButton m_bu_sync_lights;
    FBButton m_bu_sync_meshes;

    FBLabel m_lb_animations;
    FBEditNumber m_ed_time_scale;
    FBEditNumber m_ed_samples_par_per_second;
    FBButton m_bu_sync_animations;
};
