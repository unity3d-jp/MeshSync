#pragma once

struct Sync
{
public:
    Sync();
    ms::ClientSettings& getClientSettings();
    float& getScaleFactor();
    bool& getAutoSync();

    void send(MQDocument doc, bool force = false);
    void import(MQDocument doc);

private:
    using MeshDataPtr = std::shared_ptr<ms::MeshData>;

    MQObject create(const ms::MeshData& data);
    void gather(MQDocument doc, MQObject obj, ms::MeshData& data);


    ms::ClientSettings m_settings;
    std::vector<MeshDataPtr> m_data;
    std::vector<std::string> m_current_objects;;
    std::vector<std::string> m_prev_objects;
    float m_scale_factor = 0.01f;
    bool m_auto_sync = false;
};
