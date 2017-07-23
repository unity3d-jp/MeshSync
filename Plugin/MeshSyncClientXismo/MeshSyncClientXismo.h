#pragma once

struct XismoSyncSettings
{
    ms::ClientSettings client_settings;
    bool auto_sync = true;
    bool weld = true;
    bool sync_camera = false;
    float scale_factor = 100.0f;
};

void msxmInitializeWidget();
XismoSyncSettings& msxmGetSettings();
void msxmForceSetDirty();
void msxmSend(bool force = false);
