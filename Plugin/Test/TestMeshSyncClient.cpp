#include "pch.h"
#include "Test.h"
#include "MeshGenerator.h"
#include "../MeshSync/MeshSync.h"
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

static bool FileToByteArray(const char *path, RawVector<char> &out)
{
    FILE *fin = fopen(path, "rb");
    if (!fin)
        return false;

    fseek(fin, 0, SEEK_END);
    out.resize_discard((size_t)ftell(fin));
    fseek(fin, 0, SEEK_SET);
    fread(&out[0], 1, out.size(), fin);
    fclose(fin);
    return true;
}


TestCase(Test_SendMesh)
{
    ms::Scene scene;

    {
        auto mesh = ms::Mesh::create();
        scene.objects.push_back(mesh);

        mesh->path = "/Test/Quad";
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
    // raw file textures
    {
        const char *raw_files[] = {
            "Texture_RGBA_u8.png",
            "Texture_RGBA_f16.exr",
        };

        ms::Scene scene;
        for (auto filename : raw_files) {
            RawVector<char> data;
            if (FileToByteArray(filename, data)) {
                auto tex = ms::Texture::create();
                scene.textures.push_back(tex);
                tex->name = filename;
                tex->format = ms::TextureFormat::RawFile;
                tex->data = std::move(data);
            }
        }
        if (!scene.textures.empty())
            Send(scene);
    }
}