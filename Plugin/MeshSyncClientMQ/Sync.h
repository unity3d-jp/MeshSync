#pragma once

struct Sync
{
public:
    Sync();
    void setClientSettings(const ms::ClientSettings& v);
    void sync(MQDocument doc);

private:
    static void gather(MQObject obj, ms::MeshData& data);

    using MeshDataPtr = std::shared_ptr<ms::MeshData>;
    std::vector<MeshDataPtr> m_data;
    std::vector<std::string> m_prev_objects;
    std::vector<std::string> m_deleted_objects;
    std::mutex m_mutex;

    ms::ClientSettings m_settings;
};
