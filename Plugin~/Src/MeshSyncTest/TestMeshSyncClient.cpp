#include "pch.h"
#include "Test.h"
#include "Common.h"
#include "MeshGenerator.h"
#include "../MeshSync/MeshSync.h"
#include "../MeshSync/MeshSyncUtils.h"
#include "Utility/TestUtility.h"
using namespace mu;


#ifdef msEnableNetwork
static ms::ClientSettings GetClientSettings()
{
    ms::ClientSettings ret;
    GetArg("server", ret.server);
    int port;
    if (GetArg("port", port))
        ret.port = (uint16_t)port;
    return ret;
}

static void Send(ms::ScenePtr scene)
{
    ms::AsyncSceneSender sender;
    sender.client_settings = GetClientSettings();
    if (sender.isServerAvaileble()) {
        sender.add(scene);
        sender.kick();
    }
}



TestCase(Test_SendMesh)
{
    ms::OSceneCacheSettings c0;
    c0.strip_unchanged = 0;
    c0.flatten_hierarchy = 0;
    c0.encoding = ms::SceneCacheEncoding::Plain;

    ms::OSceneCacheSettings c1;
    c1.flatten_hierarchy = 0;

    ms::OSceneCacheSettings c2;
    c2.flatten_hierarchy = 0;
    c2.encoder_settings.zstd.compression_level = 100;

    ms::AsyncSceneCacheWriter writer0, writer1, writer2;
    writer0.open("wave_c0.sc", c0);
    writer1.open("wave_c1.sc", c1);
    writer2.open("wave_c2.sc", c2);

    for (int i = 0; i < 8; ++i) {
        auto scene = ms::Scene::create();

        auto mesh = ms::Mesh::create();
        scene->entities.push_back(mesh);

        mesh->path = "/Test/Wave";
        mesh->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_NORMALS, true);
        mesh->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_TANGENTS, true);


        SharedVector<float3>& points = mesh->points;
        SharedVector<tvec2<float>>& uv = mesh->m_uv[0];
        SharedVector<int>& counts = mesh->counts;
        SharedVector<int>& indices = mesh->indices;
        SharedVector<int>& materialIDs = mesh->material_ids;

        GenerateWaveMesh(counts, indices, points, uv, 2.0f, 1.0f, 32, 30.0f * mu::DegToRad * i);
        materialIDs.resize(counts.size(), 0);
        mesh->setupDataFlags();


        writer0.time = writer1.time = writer2.time = 0.5f * i;

        writer0.geometries.emplace_back(std::static_pointer_cast<ms::Transform>(mesh->clone(true)));
        writer0.kick();

        writer1.geometries.emplace_back(std::static_pointer_cast<ms::Transform>(mesh->clone(true)));
        writer1.kick();

        writer2.geometries.emplace_back(std::static_pointer_cast<ms::Transform>(mesh->clone(true)));
        writer2.kick();

        Send(scene);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

TestCase(Test_SceneCacheRead)
{
    ms::ISceneCacheSettings iscs;
    iscs.enable_diff = false;
    auto isc = ms::OpenISceneCacheFile("wave_c2.sc", iscs);
    Expect(isc);
    if (!isc)
        return;

    auto range = isc->getTimeRange();
    float step = 0.1f;
    for (float t = range.start; t < range.end; t += step) {
        auto scene = isc->getByTime(t, true);
        if (!scene)
            break;
        Send(scene);

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

TestCase(Test_Animation)
{
    auto scene = ms::Scene::create();
    {
        auto node = ms::Mesh::create();
        scene->entities.push_back(node);

        node->path = "/Test/Animation";
        node->position = { 0.0f, 0.0f, 0.0f };
        node->rotation = quatf::identity();
        node->scale = { 1.0f, 1.0f, 1.0f };
        GenerateIcoSphereMesh(node->counts, node->indices, node->points, node->m_uv[0], 0.5f, 1);

        node->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_NORMALS, true);
        node->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_TANGENTS, true);
    }
    {
        auto clip = ms::AnimationClip::create();
        scene->assets.push_back(clip);

        auto anim = ms::TransformAnimation::create();
        clip->addAnimation(anim);

        anim->path = "/Test/Animation";
        anim->translation.push_back({ 0.0f, {0.0f, 0.0f, 0.0f} });
        anim->translation.push_back({ 1.0f, {1.0f, 0.0f, 0.0f} });
        anim->translation.push_back({ 2.0f, {1.0f, 1.0f, 0.0f} });
        anim->translation.push_back({ 3.0f, {1.0f, 1.0f, 1.0f} });

        anim->rotation.push_back({ 0.0f, ms::rotate_x(0.0f * mu::DegToRad) });
        anim->rotation.push_back({ 1.0f, ms::rotate_x(90.0f * mu::DegToRad) });
        anim->rotation.push_back({ 2.0f, ms::rotate_x(180.0f * mu::DegToRad) });
        anim->rotation.push_back({ 3.0f, ms::rotate_x(270.0f * mu::DegToRad) });

        anim->scale.push_back({ 0.0f, {1.0f, 1.0f, 1.0f} });
        anim->scale.push_back({ 1.0f, {2.0f, 2.0f, 2.0f} });
        anim->scale.push_back({ 2.0f, {1.0f, 1.0f, 1.0f} });
        anim->scale.push_back({ 3.0f, {2.0f, 2.0f, 2.0f} });
    }
    Send(scene);
}

TestCase(Test_MeshMerge)
{
    std::shared_ptr<ms::Scene> scene = ms::Scene::create();

    std::shared_ptr<ms::Mesh> mesh = ms::Mesh::create();
    scene->entities.push_back(mesh);
    {
        mesh->path = "/Test/Merge";
        mesh->position = { 0.0f, 0.0f, 0.0f };
        mesh->rotation = quatf::identity();
        mesh->scale = { 1.0f, 1.0f, 1.0f };

        GenerateWaveMesh(mesh->counts, mesh->indices, mesh->points, mesh->m_uv[0], 2.0f, 1.0f, 16, 90.0f * mu::DegToRad);
        mesh->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_NORMALS, true);
        mesh->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_TANGENTS, true);
        mesh->material_ids.resize(mesh->counts.size(), 0);
    }

    {
        std::shared_ptr<ms::Mesh> sphere = ms::Mesh::create();
        GenerateIcoSphereMesh(sphere->counts, sphere->indices, sphere->points, sphere->m_uv[0], 0.5f, 1);
        sphere->material_ids.resize(sphere->counts.size(), 1);
        sphere->transformMesh(mu::translate(float3{ 0.0f, 1.5f, 0.0f }));

        mesh->mergeMesh(*sphere);
    }
    Send(scene);
}

TestCase(Test_Points)
{
    Random rand;

    std::shared_ptr<ms::Scene> scene = ms::Scene::create();
    {
        std::shared_ptr<ms::Mesh> node = ms::Mesh::create();
        scene->entities.push_back(node);

        node->path = "/Test/PointMesh";
        node->position = { 0.0f, 0.0f, 0.0f };
        node->rotation = quatf::identity();
        node->scale = { 1.0f, 1.0f, 1.0f };
        node->visibility = { false, true, true };
        GenerateIcoSphereMesh(node->counts, node->indices, node->points, node->m_uv[0], 0.1f, 1);
        node->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_NORMALS, true);
        node->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_TANGENTS, true);
        {
            ms::Variant test("test", ms::float4::one());
            node->addUserProperty(std::move(test));
        }
    }
    {
        std::shared_ptr<ms::Points> node = ms::Points::create();
        scene->entities.push_back(node);

        node->path = "/Test/PointsT";
        node->reference = "/Test/PointMesh";
        node->position = { -2.5f, 0.0f, 0.0f };

        int N = 100;
        node->points.resize_discard(N);
        for (int i = 0; i < N;++i) {
            node->points[i] = { rand.f11(), rand.f11(), rand.f11() };
        }
        node->setupPointsDataFlags();
    }
    {
        std::shared_ptr<ms::Points> node = ms::Points::create();
        scene->entities.push_back(node);

        node->path = "/Test/PointsTR";
        node->reference = "/Test/PointMesh";
        node->position = { 0.0f, 0.0f, 0.0f };

        const int N = 100;
        node->points.resize_discard(N);
        node->rotations.resize_discard(N);
        for (int i = 0; i < N; ++i) {
            node->points[i] = { rand.f11(), rand.f11(), rand.f11() };
            node->rotations[i] = rotate(rand.v3n(), rand.f11() * mu::PI);
        }
        node->setupPointsDataFlags();
    }
    {
        std::shared_ptr<ms::Points> node = ms::Points::create();
        scene->entities.push_back(node);

        node->path = "/Test/PointsTRS";
        node->reference = "/Test/PointMesh";
        node->position = { 2.5f, 0.0f, 0.0f };

        int N = 100;
        node->points.resize_discard(N);
        node->rotations.resize_discard(N);
        node->scales.resize_discard(N);
        node->colors.resize_discard(N);
        node->velocities.resize_discard(N);
        for (int i = 0; i < N; ++i) {
            node->points[i] = { rand.f11(), rand.f11(), rand.f11() };
            node->rotations[i] = rotate(rand.v3n(), rand.f11() * mu::PI);
            node->scales[i] = { rand.f01(), rand.f01(), rand.f01() };
            node->colors[i] = { rand.f01(), rand.f01(), rand.f01() };
            node->velocities[i] = float3{ rand.f11(), rand.f11(), rand.f11() } *0.1f;
        }
        node->setupPointsDataFlags();

    }
    Send(scene);

    // animation
    {
        const int F = 20;
        std::shared_ptr<ms::Points> node = std::static_pointer_cast<ms::Points>(scene->entities.back());

        for (int fi = 0; fi < F; ++fi) {
            for (int i = 0; i < node->points.size(); ++i)
                node->points[i] += node->velocities[i];

            Send(scene);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

TestCase(Test_SendTexture) {
    auto gen_id = []() {
        static int id_seed = 0;
        return ++id_seed;
    };

    // raw file textures
    {
        const char *raw_files[] = {
            "Texture_RGBA_u8.png",
            "Texture_RGBA_f16.exr",
        };

        std::shared_ptr<ms::Scene> scene = ms::Scene::create();
        for (const char* filename : raw_files) {
            auto tex = ms::Texture::create();
            if (tex->readFromFile(filename)) {
                scene->assets.push_back(tex);
                tex->id = gen_id();
            }
        }
        if (!scene->assets.empty())
            Send(scene);
    }

    {
        std::shared_ptr<ms::Scene> scene = ms::Scene::create();

        const int width = 512;
        const int height = 512;
        {
            // Ru8
            const unorm8 black{ 0.0f };
            const unorm8 white{ 1.0f };
            scene->assets.push_back(TestUtility::CreateCheckerImageTexture<unorm8>(black, white, width, height, gen_id(), "Ru8"));
        }
        {
            // RGu8
            const unorm8x2 black{ 0.0f, 0.0f };
            const unorm8x2 white{ 1.0f, 1.0f };
            scene->assets.push_back(TestUtility::CreateCheckerImageTexture<unorm8x2>(black, white, width, height, gen_id(), "RGu8"));
        }
        {
            // RGBAu8
            const unorm8x3 black{ 0.0f, 0.0f, 0.0f };
            const unorm8x3 white{ 1.0f, 1.0f, 1.0f };
            scene->assets.push_back(TestUtility::CreateCheckerImageTexture<unorm8x3>(black, white, width, height, gen_id(), "RGBu8"));
        }
        {
            // RGBAu8
            const unorm8x4 black{ 0.0f, 0.0f, 0.0f, 1.0f };
            const unorm8x4 white{ 1.0f, 1.0f, 1.0f, 1.0f };
            scene->assets.push_back(TestUtility::CreateCheckerImageTexture<unorm8x4>(black, white, width, height, gen_id(), "RGBAu8"));
        }
        {
            // RGBAf16
            const half4 black{ 0.0f, 0.0f, 0.0f, 1.0f };
            const half4 white{ 1.0f, 1.0f, 1.0f, 1.0f };
            scene->assets.push_back(TestUtility::CreateCheckerImageTexture<half4>(black, white, width, height, gen_id(), "RGBAf16"));
        }
        {
            // RGBAf32
            const float4 black{ 0.0f, 0.0f, 0.0f, 1.0f };
            const float4 white{ 1.0f, 1.0f, 1.0f, 1.0f };
            scene->assets.push_back(TestUtility::CreateCheckerImageTexture<float4>(black, white, width, height, gen_id(), "RGBAf32"));
        }

        // material
        {
            auto mat = ms::Material::create();
            scene->assets.push_back(mat);
            mat->name = "TestMaterial1";
            mat->id = 0;
            ms::StandardMaterial& standardMaterial = ms::AsStandardMaterial(*mat);
            standardMaterial.setColor({ 0.3f, 0.3f, 0.5f, 1.0f });
            standardMaterial.setEmissionColor({ 0.7f, 0.1f, 0.2f, 1.0f });
            standardMaterial.setMetallic(0.2f);
            standardMaterial.setSmoothness(0.8f);
            standardMaterial.setColorMap(1);
            standardMaterial.setMetallicMap(5);
            standardMaterial.setEmissionMap(4);

            standardMaterial.addKeyword({ "_EMISSION", true });
            standardMaterial.addKeyword({ "_INVALIDKEYWORD", true });
        }
        Send(scene);
    }
}



static const int Frequency = 48000;
static const int Channels = 1;

template<class T >
static void GenerateAudioSample(T *dst, int n)
{
    for (int i = 0; i < n; ++i) {
        float s = std::pow(static_cast<float>(n - i) / n, 0.5f);
        dst[i] = std::sin((static_cast<float>(i) * 1.5f * ms::DegToRad)) * s;
    }
}

static ms::AudioPtr CreateAudioAsset(const char *name, ms::AudioFormat fmt, int id)
{
    std::shared_ptr<ms::Audio> a = ms::Audio::create();
    a->id = id;
    a->name = name;
    a->format = fmt;
    a->frequency = Frequency;
    a->channels = Channels;

    const auto samples = a->allocate(Frequency / 2); // 0.5 sec
    switch (fmt) {
    case ms::AudioFormat::U8:
        GenerateAudioSample(static_cast<unorm8n*>(samples), a->getSampleLength() * Channels);
        break;
    case ms::AudioFormat::S16:
        GenerateAudioSample(static_cast<snorm16*>(samples), a->getSampleLength() * Channels);
        break;
    case ms::AudioFormat::S24:
        GenerateAudioSample(static_cast<snorm24*>(samples), a->getSampleLength() * Channels);
        break;
    case ms::AudioFormat::S32:
        GenerateAudioSample(static_cast<snorm32*>(samples), a->getSampleLength() * Channels);
        break;
    case ms::AudioFormat::F32:
        GenerateAudioSample(static_cast<float*>(samples), a->getSampleLength() * Channels);
        break;
    default:
        break;
    }

    std::string filename = name;
    filename += ".wav";
    a->exportAsWave(filename.c_str());
    return a;
}

static ms::AudioPtr CreateAudioFileAsset(const char *path, const int id)
{
    std::shared_ptr<ms::Audio> a = ms::Audio::create();
    a->id = id;
    if (a->readFromFile(path))
        return a;
    return nullptr;
}

TestCase(Test_Audio)
{
    int ids = 0;
    std::shared_ptr<ms::Scene> scene = ms::Scene::create();
    scene->assets.push_back(CreateAudioAsset("audio_u8", ms::AudioFormat::U8, ids++));
    scene->assets.push_back(CreateAudioAsset("audio_s16", ms::AudioFormat::S16, ids++));
    scene->assets.push_back(CreateAudioAsset("audio_s24", ms::AudioFormat::S24, ids++));
    scene->assets.push_back(CreateAudioAsset("audio_s32", ms::AudioFormat::S32, ids++));
    scene->assets.push_back(CreateAudioAsset("audio_f32", ms::AudioFormat::F32, ids++));
    if (ms::AudioPtr afa = CreateAudioFileAsset("explosion1.wav", ids++))
        scene->assets.push_back(afa);
    Send(scene);
}

TestCase(Test_FileAsset)
{
    std::shared_ptr<ms::Scene> scene = ms::Scene::create();

    // file asset
    {
        std::shared_ptr<ms::FileAsset> as = ms::FileAsset::create();
        if (as->readFromFile("pch.h"))
            scene->assets.push_back(as);
    }
    Send(scene);
}


TestCase(Test_Query)
{
    ms::Client client(GetClientSettings());
    if (!client.isServerAvailable()) {
        const std::string& log = client.getErrorMessage();
        Print("Server not available. error log: %s\n", log.c_str());
        return;
    }

    auto send_query_impl = [&](ms::QueryMessage::QueryType qt, const char *query_name) {
        ms::QueryMessage query;
        query.query_type = qt;
        ms::ResponseMessagePtr response = client.send(query);

        Print("query: %s\n", query_name);
        Print("response:\n");
        if (response) {
            for (auto& t : response->text)
                Print("  %s\n", t.c_str());
        }
        else {
            Print("  no response. error log: %s\n", client.getErrorMessage().c_str());
        }
    };

#define SendQuery(Q) send_query_impl(ms::QueryMessage::QueryType::Q, #Q)
    SendQuery(PluginVersion);
    SendQuery(ProtocolVersion);
    SendQuery(HostName);
    SendQuery(RootNodes);
    SendQuery(AllNodes);
#undef SendQuery
}
#endif // msEnableNetwork
