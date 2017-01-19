#pragma once

#include "MeshSync/msClient.h"

struct MQSync
{
public:
    MQSync();
    ~MQSync();
    ms::ClientSettings& getClientSettings();
    float& getScaleFactor();
    bool& getAutoSync();

    void clear();
    void flushPendingRequests(MQDocument doc);
    void sendMesh(MQDocument doc, bool force = false);
    void importMeshes(MQDocument doc);

private:
    using MeshPtr = std::shared_ptr<ms::MeshData>;
    using Meshes = std::vector<MeshPtr>;

    bool isAsyncSendInProgress();
    void waitAsyncSend();
    MQObject findMQObject(MQDocument doc, const char *name);
    MQObject createObject(const ms::MeshData& data, const char *name);
    void extractMeshData(MQDocument doc, MQObject obj, ms::MeshData& data);

    using ExportObjects = std::vector<MeshPtr>;
    using ImportObjects = std::map<int, MeshPtr>;
    using ExistRecords = std::map<std::string, bool>;

    ms::ClientSettings m_settings;
    float m_scale_factor = 0.005f;
    bool m_auto_sync = false;

    ExportObjects m_export_objects;
    ImportObjects m_import_objects;
    ExistRecords m_exist_record;
    std::future<void> m_future_send;
    bool m_pending_send_mesh = false;
};
