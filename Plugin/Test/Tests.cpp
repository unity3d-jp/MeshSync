#include "pch.h"
#include "MeshSync/msServer.h"
#include "MeshSync/msClient.h"

int main(int argc, char *argv[])
{
    ms::Server server(ms::ServerSettings{});
    server.start();

    {
        ms::EditData data;
        data.obj_path = "hoge";
        data.points = {
            { -1.0f, 0.0f, -1.0f },
            {  1.0f, 0.0f, -1.0f },
            {  1.0f, 0.0f,  1.0f },
            { -1.0f, 0.0f,  1.0f },
        };
        data.indices = { 0, 1, 2, 0, 2, 3 };

        ms::Client client(ms::ClientSettings{});
        client.sendEdit(data);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    server.processEvents([](const ms::EventData& data) {
        printf("hoge\n");
    });
}
