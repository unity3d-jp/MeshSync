#pragma once

struct Sync
{
public:
    Sync();
    ms::ClientSettings& getClientSettings();
    void sync(MQDocument doc);

private:
    static void gather(MQDocument doc, MQObject obj, ms::MeshData& data);

    using MeshDataPtr = std::shared_ptr<ms::MeshData>;
    using Fence = std::future<void>;
    std::vector<MeshDataPtr> m_data;
    std::vector<std::string> m_current_objects, m_prev_objects;
    std::vector<Fence> m_fences;
    std::mutex m_mutex;

    ms::ClientSettings m_settings;
};
