#pragma once

struct Sync
{
public:
    void gather(MQDocument doc);
    static void gather(MQObject obj, ms::MeshData& data);

    std::vector<ms::MeshData> data;
    std::vector<std::string> prev_objects;
    std::vector<std::string> deleted_objects;
};
