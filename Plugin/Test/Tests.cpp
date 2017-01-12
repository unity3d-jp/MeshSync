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
        data.obj_path = "Root/Child";
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
        server->processEvents([](const ms::EventData& data) {
            printf("break here\n");
        });
    }
}

void Test_GenNormals()
{
    RawVector<float3> points = {
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f },
        { 2.0f, 0.0f, 0.0f },
        { 2.0f, 0.0f, 1.0f },
    };
    RawVector<int> indices = {
        0, 1, 3, 2,
        2, 3, 5, 4,
    };
    RawVector<int> counts = {
        4, 4,
    };
    RawVector<int> offsets = {
        0, 4,
    };

    RawVector<float3> normals(indices.size());
    mu::GenerateNormalsWithThreshold(normals, points, counts, offsets, indices, 10.0f);
}

int main(int argc, char *argv[])
{
    //Test_Sync(false);
    Test_GenNormals();
}
