#pragma once

#include "MeshSync/msClient.h"

struct Sync
{
public:
    Sync();
    ~Sync();
    ms::ClientSettings& getClientSettings();
    float& getScaleFactor();
    bool& getAutoSync();

    void send(MQDocument doc, bool force = false);
    void import(MQDocument doc);

private:
    using MeshPtr = std::shared_ptr<ms::MeshData>;
    using Meshes = std::vector<MeshPtr>;

    void waitAsyncSend();
    MQObject findObject(MQDocument doc, const char *name);
    MQObject createObject(const ms::MeshData& data, const char *name);
    void gather(MQDocument doc, MQObject obj, ms::MeshData& data);

    ms::ClientSettings m_settings;

    std::vector<MeshPtr> m_data;
    std::vector<std::string> m_current_objects;
    std::vector<std::string> m_prev_objects;
    std::future<void> m_future_send;
    float m_scale_factor = 0.01f;
    bool m_auto_sync = false;
};
