#include "pch.h"
#include "Test.h"
#include "MeshGenerator.h"
#include "../MeshSync//msClient.h"
using namespace mu;

TestCase(Test_SendMesh)
{
    ms::Scene scene;

    {
        auto mesh = ms::Mesh::create();
        scene.objects.push_back(mesh);

        mesh->path = "/pmsMesh";

        auto& points = mesh->points;
        auto& uv = mesh->uv0;
        auto& counts = mesh->counts;
        auto& indices = mesh->indices;
        auto& mids = mesh->material_ids;

        points.push_back({ 0.0f, 0.0f, 0.0f });
        points.push_back({ 0.0f, 0.0f, 1.0f });
        points.push_back({ 1.0f, 0.0f, 1.0f });
        points.push_back({ 1.0f, 0.0f, 0.0f });

        uv.push_back({ 0.0f, 0.0f });
        uv.push_back({ 0.0f, 1.0f });
        uv.push_back({ 1.0f, 1.0f });
        uv.push_back({ 1.0f, 0.0f });

        counts.push_back(4);
        mids.push_back(0);
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(3);
    }

    {
        ms::ClientSettings settings;
        ms::Client client(settings);

        ms::SetMessage mes;
        mes.scene = std::move(scene);
        client.send(mes);
    }
}

TestCase(Test_SendTexture)
{

}