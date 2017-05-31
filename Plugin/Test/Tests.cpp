#include "pch.h"
#include "MeshSync/msServer.h"
#include "MeshSync/msClient.h"

using namespace mu;

using ns = uint64_t;

static ns now()
{
    using namespace std::chrono;
    return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}


void Test_Sync(bool create_server)
{
    std::unique_ptr<ms::Server> server;

    if (create_server) {
        server.reset(new ms::Server(ms::ServerSettings{}));
        server->start();
    }

    {
        auto mesh = new ms::Mesh();
        mesh->path = "/Root/Child";
        mesh->points = {
            { -1.0f, 0.0f, -1.0f },
            { -1.0f, 0.0f,  1.0f },
            {  1.0f, 0.0f,  1.0f },
            {  1.0f, 0.0f, -1.0f },
        };
        mesh->indices = { 0, 1, 2, 0, 2, 3 };

        ms::Client client(ms::ClientSettings{});
        ms::SetMessage mes;
        mes.scene.meshes.emplace_back(mesh);
        client.send(mes);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (create_server) {
        server->processMessages([](ms::MessageType type, const ms::Message& mes) {
            printf("break here\n");
        });
    }
}

void Test_Get()
{
    ms::Client client(ms::ClientSettings{});

    ms::GetMessage gdata;
    if (auto data = client.send(gdata)) {
        for (auto& a : data->meshes) {
            printf("");
        }
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
    RawVector<int> materialIDs = {
        0, 1, 2, 3
    };

    RawVector<float2> uv_flattened(indices.size());
    for (int i = 0; i < indices.size(); ++i) {
        uv_flattened[i] = uv[indices[i]];
    }

    mu::MeshRefiner refiner;
    refiner.prepare(counts, indices, points);
    refiner.uv = uv_flattened;
    refiner.split_unit = 8;
    refiner.genNormals(40.0f);
    refiner.genTangents();
    refiner.refine(false);
    refiner.genSubmesh(materialIDs);
}

void MatrixSwapHandedness()
{
    quatf rot1 = rotate(normalize(float3{0.15f, 0.3f, 0.6f}), 60.0f);
    quatf rot2 = swap_handedness(rot1);
    float4x4 mat1 = to_float4x4(rot1);
    float4x4 mat2 = to_float4x4(rot2);
    float4x4 mat3 = swap_handedness(mat1);
    float4x4 imat1 = invert(mat1);
    float4x4 imat2 = invert(mat2);
    float4x4 imat3 = swap_handedness(imat1);

    bool r1 = near_equal(mat2, mat3);
    bool r2 = near_equal(imat2, imat3);
    printf("");
}

void RayTrianglesIntersectionTest()
{
    RawVector<float3> vertices;
    RawVector<float3> vertices_flattened;
    RawVector<int> indices;
    RawVector<int> hit;

    const int seg = 1000;
    const int num_try = 1;

    vertices.resize(seg * seg);
    for (int yi = 0; yi < seg; ++yi) {
        for (int xi = 0; xi < seg; ++xi) {
            float3 v = { xi - (float)seg*0.5f, 0.0f, yi - (float)seg*0.5f};
            vertices[yi * seg + xi] = v;
        }
    }

    indices.resize((seg - 1) * (seg - 1) * 6);
    for (int yi = 0; yi < seg - 1; ++yi) {
        for (int xi = 0; xi < seg - 1; ++xi) {
            int i = yi * (seg - 1) + xi;
            indices[i * 6 + 0] = seg* yi + xi;
            indices[i * 6 + 1] = seg* (yi + 1) + xi;
            indices[i * 6 + 2] = seg* (yi + 1) + (xi + 1);
            indices[i * 6 + 3] = seg* yi + xi;
            indices[i * 6 + 4] = seg* (yi + 1) + (xi + 1);
            indices[i * 6 + 5] = seg * yi + (xi + 1);
        }
    }

    vertices_flattened.resize(indices.size());
    for (int i = 0; i < indices.size(); ++i) {
        vertices_flattened[i] = vertices[indices[i]];
    }

    float3 ray_pos = { 0.0f, 1.0f, 0.0f };
    float3 ray_dir = { 0.0f, -1.0f, 0.0f };

    auto print = [&hit]() {
        printf("    hits: ");
        for (int i = 0; i < hit.size(); ++i) {
            printf("%d ", hit[i]);
        }
        printf("\n");
    };
    printf("RayTrianglesIntersectionTest()\n");

    auto s1_begin = now();
    for (int i = 0; i < num_try; ++i) {
        hit.resize(indices.size() / 3);
        int num_hits = RayTrianglesIntersection_Generic(ray_pos, ray_dir, vertices.data(), indices.data(), indices.size() / 3, hit.data());
        hit.resize(num_hits);
    }
    auto s1_end = now();

    print();

    auto s2_begin = now();
    for (int i = 0; i < num_try; ++i) {
        hit.resize(indices.size() / 3);
        int num_hits = RayTrianglesIntersection_ISPC(ray_pos, ray_dir, vertices.data(), indices.data(), indices.size() / 3, hit.data());
        hit.resize(num_hits);
    }
    auto s2_end = now();

    print();

    auto s3_begin = now();
    for (int i = 0; i < num_try; ++i) {
        hit.resize(indices.size() / 3);
        int num_hits = RayTrianglesIntersection_Generic(ray_pos, ray_dir, vertices_flattened.data(), vertices_flattened.size() / 3, hit.data());
        hit.resize(num_hits);
    }
    auto s3_end = now();

    print();

    auto s4_begin = now();
    for (int i = 0; i < num_try; ++i) {
        hit.resize(indices.size() / 3);
        int num_hits = RayTrianglesIntersection_ISPC(ray_pos, ray_dir, vertices_flattened.data(), vertices_flattened.size() / 3, hit.data());
        hit.resize(num_hits);
    }
    auto s4_end = now();

    print();

    printf("    %f : %f\n    %f : %f\n",
        float(s1_end - s1_begin) / 1000000.0f,
        float(s2_end - s2_begin) / 1000000.0f,
        float(s3_end - s3_begin) / 1000000.0f,
        float(s4_end - s4_begin) / 1000000.0f);

}

int main(int argc, char *argv[])
{
    //Test_Sync(false);
    //Test_Get();
    //Test_GenNormals();
    //MatrixSwapHandedness();
    RayTrianglesIntersectionTest();
}
