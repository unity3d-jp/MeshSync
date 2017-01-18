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
        ms::MeshData data;
        data.path = "/Root/Child";
        data.points = {
            { -1.0f, 0.0f, -1.0f },
            { -1.0f, 0.0f,  1.0f },
            {  1.0f, 0.0f,  1.0f },
            {  1.0f, 0.0f, -1.0f },
        };
        data.indices = { 0, 1, 2, 0, 2, 3 };

        ms::Client client(ms::ClientSettings{});
        client.send(data);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (create_server) {
        server->processMessages([](const ms::MessageData& data) {
            printf("break here\n");
        });
    }
}

void Test_Get()
{
    ms::Client client(ms::ClientSettings{});

    ms::GetData gdata;
    auto data = client.send(gdata);
    for(auto& a : data) {
        printf("");
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
    RawVector<int> offsets = {
        0, 4, 8, 12
    };

    RawVector<float2> uv_flattened(indices.size());
    for (int i = 0; i < indices.size(); ++i) {
        uv_flattened[i] = uv[indices[i]];
    }

    mu::TopologyRefiner refiner;
    refiner.prepare(counts, offsets, indices, points);
    refiner.uv = uv_flattened;
    refiner.split_unit = 4;
    refiner.genNormals(40.0f);
    refiner.genTangents();
    refiner.refine();
}

int main(int argc, char *argv[])
{
    //Test_Sync(false);
    //Test_Get();
    Test_GenNormals();
}
