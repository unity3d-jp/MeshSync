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


TestCase(Test_SendMesh)
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


template<class color_t>
void CreateCheckerImage(RawVector<char>& dst, color_t black, color_t white, int width, int height)
{
    int num_pixels = width * height;
    int checker_size = 8;
    dst.resize_discard(num_pixels * sizeof(color_t));
    color_t *data = (color_t*)dst.data();
    for (int iy = 0; iy < height; ++iy) {
        for (int ix = 0; ix < width; ++ix) {
            bool cy = (iy / checker_size) % 2 == 0;
            bool cx = (ix / checker_size) % 2 == 0;
            if (cy)
                *data++ = cx ? white : black;
            else
                *data++ = cx ? black : white;
        }
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
            if (ms::FileToByteArray(filename, data)) {
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

    {
        ms::Scene scene;

        const int width = 512;
        const int height = 512;
        {
            // RGBAu8
            tvec4<unorm8> black{ 0.0f, 0.0f, 0.0f, 1.0f };
            tvec4<unorm8> white{ 1.0f, 1.0f, 1.0f, 1.0f };

            RawVector<char> data;
            CreateCheckerImage(data, black, white, width, height);

            auto tex = ms::Texture::create();
            scene.textures.push_back(tex);
            tex->name = "RGBAu8";
            tex->format = ms::TextureFormat::RGBAu8;
            tex->width = width;
            tex->height = height;
            tex->data = std::move(data);
        }
        {
            // RGBAf16
            half4 black{ 0.0f, 0.0f, 0.0f, 1.0f };
            half4 white{ 1.0f, 1.0f, 1.0f, 1.0f };

            RawVector<char> data;
            CreateCheckerImage(data, black, white, width, height);

            auto tex = ms::Texture::create();
            scene.textures.push_back(tex);
            tex->name = "RGBAf16";
            tex->format = ms::TextureFormat::RGBAf16;
            tex->width = width;
            tex->height = height;
            tex->data = std::move(data);
        }
        {
            // RGBAf32
            float4 black{ 0.0f, 0.0f, 0.0f, 1.0f };
            float4 white{ 1.0f, 1.0f, 1.0f, 1.0f };

            RawVector<char> data;
            CreateCheckerImage(data, black, white, width, height);

            auto tex = ms::Texture::create();
            scene.textures.push_back(tex);
            tex->name = "RGBAf32";
            tex->format = ms::TextureFormat::RGBAf32;
            tex->width = width;
            tex->height = height;
            tex->data = std::move(data);
        }

        Send(scene);
    }
}