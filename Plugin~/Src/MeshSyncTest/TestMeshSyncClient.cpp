#include "pch.h"
#include "Test.h"
#include "Common.h"
#include "Utility/TestUtility.h"
#include "Utility/MeshGenerator.h"

#include "MeshSync/SceneGraph/msAnimation.h"
#include "MeshSync/SceneGraph/msMaterial.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshSync/SceneGraph/msPoints.h"
#include "MeshSync/SceneGraph/msScene.h"

#include "MeshSync/SceneCache/msSceneCacheInputSettings.h"
#include "MeshSync/SceneCache/msSceneCacheOutputSettings.h"
#include "MeshSync/SceneCache/msSceneCacheEncoding.h"
#include "MeshSync/SceneCache/msSceneCacheWriter.h" //SceneCacheWriter

#include "MeshSync/Utility/msMaterialExt.h"     //standardMaterial
#include "MeshSync/SceneCache/msSceneCacheInputFile.h"

using namespace mu;

TestCase(Test_SendMesh) {
    ms::SceneCacheOutputSettings c0;
    c0.exportSettings.stripUnchanged = 0;
    c0.exportSettings.flattenHierarchy = 0;
    c0.exportSettings.encoding = ms::SceneCacheEncoding::Plain;

    ms::SceneCacheOutputSettings c1;
    c1.exportSettings.flattenHierarchy = 0;

    ms::SceneCacheOutputSettings c2;
    c2.exportSettings.flattenHierarchy = 0;
    c2.exportSettings.encoderSettings.zstd.compressionLevel = 100;

    ms::SceneCacheWriter writer0, writer1, writer2;
    writer0.Open("wave_c0.sc", c0);
    writer1.Open("wave_c1.sc", c1);
    writer2.Open("wave_c2.sc", c2);

    for (int i = 0; i < 8; ++i) {
        std::shared_ptr<ms::Scene> scene = ms::Scene::create();

        std::shared_ptr<ms::Mesh> mesh = ms::Mesh::create();
        scene->entities.push_back(mesh);

        mesh->path = "/Test/Wave";
        mesh->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_NORMALS, true);
        mesh->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_TANGENTS, true);


        SharedVector<float3>& points = mesh->points;
        SharedVector<tvec2<float>>* uv = mesh->m_uv;
        SharedVector<int>& counts = mesh->counts;
        SharedVector<int>& indices = mesh->indices;
        SharedVector<int>& materialIDs = mesh->material_ids;

        MeshGenerator::GenerateWaveMesh(counts, indices, points, uv, 2.0f, 1.0f, 32, 30.0f * mu::DegToRad * i);
        materialIDs.resize(counts.size(), 0);
        mesh->setupDataFlags();


        const float writerTime = 0.5f * i;
        writer0.SetTime(writerTime);
        writer1.SetTime(writerTime);
        writer2.SetTime(writerTime);

        writer0.geometries.emplace_back(std::static_pointer_cast<ms::Transform>(mesh->clone(true)));
        writer0.kick();

        writer1.geometries.emplace_back(std::static_pointer_cast<ms::Transform>(mesh->clone(true)));
        writer1.kick();

        writer2.geometries.emplace_back(std::static_pointer_cast<ms::Transform>(mesh->clone(true)));
        writer2.kick();

        TestUtility::Send(scene);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

TestCase(Test_SceneCacheRead)
{
    ms::SceneCacheInputSettings iscs;
    iscs.enableDiff = false;
    ms::SceneCacheInputFilePtr isc = ms::SceneCacheInputFile::Open("wave_c2.sc", iscs);
    Expect(isc);
    if (!isc)
        return;

    const ms::TimeRange range = isc->GetTimeRangeV();
    const float step = 0.1f;
    for (float t = range.start; t < range.end; t += step) {
        const ms::ScenePtr scene = isc->LoadByTimeV(t, true);
        if (!scene)
            break;
        TestUtility::Send(scene);

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
}

TestCase(Test_Animation)
{
    std::shared_ptr<ms::Scene> scene = ms::Scene::create();
    {
        std::shared_ptr<ms::Mesh> node = ms::Mesh::create();
        scene->entities.push_back(node);

        node->path = "/Test/Animation";
        node->position = { 0.0f, 0.0f, 0.0f };
        node->rotation = quatf::identity();
        node->scale = { 1.0f, 1.0f, 1.0f };
        MeshGenerator::GenerateIcoSphereMesh(node->counts, node->indices, node->points, node->m_uv[0], 0.5f, 1);

        node->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_NORMALS, true);
        node->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_TANGENTS, true);
    }
    {
        std::shared_ptr<ms::AnimationClip> clip = ms::AnimationClip::create();
        scene->assets.push_back(clip);

        std::shared_ptr<ms::TransformAnimation> anim = ms::TransformAnimation::create();
        clip->addAnimation(anim);

        anim->path = "/Test/Animation";
        anim->translation.push_back({ 0.0f, {0.0f, 0.0f, 0.0f} });
        anim->translation.push_back({ 1.0f, {1.0f, 0.0f, 0.0f} });
        anim->translation.push_back({ 2.0f, {1.0f, 1.0f, 0.0f} });
        anim->translation.push_back({ 3.0f, {1.0f, 1.0f, 1.0f} });

        anim->rotation.push_back({ 0.0f, mu::rotate_x(0.0f * mu::DegToRad) });
        anim->rotation.push_back({ 1.0f, mu::rotate_x(90.0f * mu::DegToRad) });
        anim->rotation.push_back({ 2.0f, mu::rotate_x(180.0f * mu::DegToRad) });
        anim->rotation.push_back({ 3.0f, mu::rotate_x(270.0f * mu::DegToRad) });

        anim->scale.push_back({ 0.0f, {1.0f, 1.0f, 1.0f} });
        anim->scale.push_back({ 1.0f, {2.0f, 2.0f, 2.0f} });
        anim->scale.push_back({ 2.0f, {1.0f, 1.0f, 1.0f} });
        anim->scale.push_back({ 3.0f, {2.0f, 2.0f, 2.0f} });
    }
    TestUtility::Send(scene);
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

        MeshGenerator::GenerateWaveMesh(mesh->counts, mesh->indices, mesh->points, mesh->m_uv, 2.0f, 1.0f, 16, 90.0f * mu::DegToRad);
        mesh->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_NORMALS, true);
        mesh->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_TANGENTS, true);
        mesh->material_ids.resize(mesh->counts.size(), 0);
    }

    {
        std::shared_ptr<ms::Mesh> sphere = ms::Mesh::create();
        MeshGenerator::GenerateIcoSphereMesh(sphere->counts, sphere->indices, sphere->points, sphere->m_uv[0], 0.5f, 1);
        sphere->material_ids.resize(sphere->counts.size(), 1);
        sphere->transformMesh(mu::translate(float3{ 0.0f, 1.5f, 0.0f }));

        mesh->mergeMesh(*sphere);
    }
    TestUtility::Send(scene);
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
        MeshGenerator::GenerateIcoSphereMesh(node->counts, node->indices, node->points, node->m_uv[0], 0.1f, 1);
        node->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_NORMALS, true);
        node->refine_settings.flags.Set(ms::MESH_REFINE_FLAG_GEN_TANGENTS, true);
        {
            ms::Variant test("test", mu::float4::one());
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
    TestUtility::Send(scene);

    // animation
    {
        const int F = 20;
        std::shared_ptr<ms::Points> node = std::static_pointer_cast<ms::Points>(scene->entities.back());

        for (int fi = 0; fi < F; ++fi) {
            for (int i = 0; i < node->points.size(); ++i)
                node->points[i] += node->velocities[i];

            TestUtility::Send(scene);
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
    const char* testTextures[] = {
        "Texture_RGBA_u8.png",
        "Texture_RGBA_f16.exr",
    };

    const uint32_t numTestTextures = sizeof(testTextures) / sizeof(testTextures[0]);
    int testTexturesID[numTestTextures];

    std::shared_ptr<ms::Scene> scene = ms::Scene::create();
    for (uint32_t i =0;i<numTestTextures;++i) {

        std::shared_ptr<ms::Texture> tex = ms::Texture::create();
        const bool canReadTex = tex->readFromFile(testTextures[i]);
        assert( canReadTex && "Test_SendTexture() failed in loading textures");
        scene->assets.push_back(tex);
        testTexturesID[i] = gen_id();
        tex->id = testTexturesID[i];
    }

    if (!scene->assets.empty())
        TestUtility::Send(scene);


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
        const int emissionTexID = gen_id();
        {
            // RGu8
            const unorm8x2 black{ 0.0f, 0.0f };
            const unorm8x2 white{ 1.0f, 1.0f };
            scene->assets.push_back(TestUtility::CreateCheckerImageTexture<unorm8x2>(black, white, width, height, emissionTexID, "RGu8"));
        }
        const int metallicTexID = gen_id();
        {
            // RGBu8
            const unorm8x3 black{ 0.0f, 0.0f, 0.0f };
            const unorm8x3 white{ 1.0f, 1.0f, 1.0f };
            scene->assets.push_back(TestUtility::CreateCheckerImageTexture<unorm8x3>(black, white, width, height, metallicTexID, "RGBu8"));
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
            std::shared_ptr<ms::Material> mat = ms::Material::create();
            scene->assets.push_back(mat);
            mat->name = "MeshSyncTest Material";
            mat->id = 0;
            ms::StandardMaterial& standardMaterial = ms::AsStandardMaterial(*mat);
            standardMaterial.setColor({ 0.3f, 0.3f, 0.5f, 1.0f });
            standardMaterial.setEmissionColor({ 0.7f, 0.1f, 0.2f, 1.0f });
            standardMaterial.setMetallic(0.2f);
            standardMaterial.setSmoothness(0.8f);
            standardMaterial.setColorMap(testTexturesID[0]);
            standardMaterial.setMetallicMap(metallicTexID);
            standardMaterial.setEmissionMap(emissionTexID);

            standardMaterial.SetDetailAlbedoMap(testTexturesID[1]);
            standardMaterial.SetUVForSecondaryMap(1);

            standardMaterial.addKeyword({ "_EMISSION", true });
            standardMaterial.addKeyword({ "_INVALIDKEYWORD", true });
        }
        TestUtility::Send(scene);
    }
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
    TestUtility::Send(scene);
}


TestCase(Test_Query)
{
    ms::Client client(TestUtility::GetClientSettings());
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

