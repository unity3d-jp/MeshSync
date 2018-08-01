#include "pch.h"
#include "Test.h"
#include "MeshGenerator.h"
#include "../MeshSync//msClient.h"
using namespace mu;

static void Send(ms::Scene& scene)
{
    for (auto& obj : scene.objects) {
        if (auto *mesh = dynamic_cast<ms::Mesh*>(obj.get())) {
            mesh->setupFlags();
        }
    }

    ms::ClientSettings settings;
    ms::Client client(settings);

    ms::SetMessage mes;
    mes.scene = std::move(scene);
    client.send(mes);
}

TestCase(Test_SendMesh)
{
    ms::Scene scene;

    {
        auto mesh = ms::Mesh::create();
        scene.objects.push_back(mesh);

        mesh->path = "/Test/Quad";
        mesh->flags.has_refine_settings = 1;
        mesh->refine_settings.flags.gen_normals = 1;
        mesh->refine_settings.flags.gen_tangents = 1;

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

    Send(scene);
}

TestCase(Test_SendMeshAnimated)
{
    for (int i = 0; i < 8; ++i) {
        ms::Scene scene;

        auto mesh = ms::Mesh::create();
        scene.objects.push_back(mesh);

        mesh->path = "/Test/Wave";
        mesh->flags.has_refine_settings = 1;
        mesh->refine_settings.flags.gen_normals = 1;
        mesh->refine_settings.flags.gen_tangents = 1;


        auto& points = mesh->points;
        auto& uv = mesh->uv0;
        auto& counts = mesh->counts;
        auto& indices = mesh->indices;
        auto& mids = mesh->material_ids;

        GenerateWaveMesh(counts, indices, points, uv, 2.0f, 1.0f, 32, 30.0f * mu::Deg2Rad * i);
        mids.resize(counts.size(), 0);

        Send(scene);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

TestCase(Test_SendTexture)
{

}