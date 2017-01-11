#include "pch.h"
#include "MeshSync/msServer.h"
#include "MeshSync/msClient.h"

int main(int argc, char *argv[])
{
    bool create_server = false;
    std::unique_ptr<ms::Server> server;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-server") == 0) {
            create_server = true;
        }
    }

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
