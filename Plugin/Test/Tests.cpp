#include "pch.h"
#include "MeshSync/msServer.h"
#include "MeshSync/msClient.h"

void Test_Sync(bool create_server)
{
    std::unique_ptr<ms::Server> server;

    if (create_server) {
        server.reset(new ms::Server(ms::ServerSettings{}));
        server->start();
    }

    {
        auto mesh = new ms::Mesh();
        mesh->path = "/Root/Child";
        mesh->points = {
            { -1.0f, 0.0f, -1.0f },
            { -1.0f, 0.0f,  1.0f },
            {  1.0f, 0.0f,  1.0f },
            {  1.0f, 0.0f, -1.0f },
        };
        mesh->indices = { 0, 1, 2, 0, 2, 3 };

        ms::Client client(ms::ClientSettings{});
        ms::SetMessage mes;
        mes.scene.meshes.emplace_back(mesh);
        client.send(mes);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (create_server) {
        server->processMessages([](ms::MessageType type, const ms::Message& mes) {
            printf("break here\n");
        });
    }
}

void Test_Get()
{
    ms::Client client(ms::ClientSettings{});

    ms::GetMessage gdata;
    if (auto data = client.send(gdata)) {
        for (auto& a : data->meshes) {
            printf("");
        }
    }
}

void Test_GenNormals()
{
    RawVector<float3> points = {
        { 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },{ 2.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f, 1.0f },{ 2.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 2.0f },{ 1.0f, 0.0f, 2.0f },{ 2.0f, 1.0f, 2.0f },
    };
    RawVector<float2> uv = {
        { 0.0f, 0.0f },{ 0.5f, 0.0f },{ 1.0f, 0.0f },
        { 0.0f, 0.5f },{ 0.5f, 0.5f },{ 1.0f, 0.5f },
        { 0.0f, 1.0f },{ 0.5f, 1.0f },{ 1.0f, 1.0f },
    };
    RawVector<int> indices = {
        0, 1, 4, 3,
        1, 2, 5, 4,
        3, 4, 7, 6,
        4, 5, 8, 7,
    };
    RawVector<int> counts = {
        4, 4, 4, 4
    };
    RawVector<int> materialIDs = {
        0, 1, 2, 3
    };

    RawVector<float2> uv_flattened(indices.size());
    for (int i = 0; i < indices.size(); ++i) {
        uv_flattened[i] = uv[indices[i]];
    }

    mu::MeshRefiner refiner;
    refiner.prepare(counts, indices, points);
    refiner.uv = uv_flattened;
    refiner.split_unit = 8;
    refiner.genNormals(40.0f);
    refiner.genTangents();
    refiner.refine(false);
    refiner.genSubmesh(materialIDs);
}

int main(int argc, char *argv[])
{
    //Test_Sync(false);
    //Test_Get();
    Test_GenNormals();
}
