#pragma once

struct Sync
{
public:
    Sync();
    void setDocument(MQDocument doc);
    ms::ClientSettings& getClientSettings();
    void send();
    void import();

private:
    MQObject create(const ms::MeshData& data);
    static void gather(MQDocument doc, MQObject obj, ms::MeshData& data);

    using MeshDataPtr = std::shared_ptr<ms::MeshData>;
    std::vector<MeshDataPtr> m_data;
    std::vector<std::string> m_current_objects, m_prev_objects;
    ms::ClientSettings m_settings;

    MQDocument m_doc;
};
