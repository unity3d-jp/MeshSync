#include "pch.h"
#include "MeshSync/msServer.h"
#include "MeshSync/msClient.h"

using namespace mu;

void Test_Indexed()
{
    std::vector<uint16_t> indices16 = { 0,1,0,2,1,3,2 };
    std::vector<uint32_t> indices32 = { 0,1,2,1,2,3 };
    std::vector<float> values = { 0.0f, 10.0f, 20.0f, 30.0f };

    IIArray<uint16_t, float> iia1 = { indices16, values };
    IIArray<uint32_t, float> iia2 = { indices32, values };

    printf("iia1: ");
    for(auto& v : iia1) { printf("%.f ", v); }
    printf("\n");

    printf("iia2: ");
    for (auto& v : iia2) { printf("%.f ", v); }
    printf("\n");
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
    refiner.genNormalsWithSmoothAngle(40.0f, false);
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
    RawVector<float> v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z;
    RawVector<int> indices;

    const int seg = 70;
    const int num_try = 4000;

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

    int num_triangles = indices.size() / 3;
    vertices_flattened.resize(indices.size());
    v1x.resize(num_triangles); v1y.resize(num_triangles); v1z.resize(num_triangles);
    v2x.resize(num_triangles); v2y.resize(num_triangles); v2z.resize(num_triangles);
    v3x.resize(num_triangles); v3y.resize(num_triangles); v3z.resize(num_triangles);
    for (int ti = 0; ti < num_triangles; ++ti) {
        auto p1 = vertices[indices[ti * 3 + 0]];
        auto p2 = vertices[indices[ti * 3 + 1]];
        auto p3 = vertices[indices[ti * 3 + 2]];
        vertices_flattened[ti * 3 + 0] = p1;
        vertices_flattened[ti * 3 + 1] = p2;
        vertices_flattened[ti * 3 + 2] = p3;
        v1x[ti] = p1.x; v1y[ti] = p1.y; v1z[ti] = p1.z;
        v2x[ti] = p2.x; v2y[ti] = p2.y; v2z[ti] = p2.z;
        v3x[ti] = p3.x; v3y[ti] = p3.y; v3z[ti] = p3.z;
    }

    float3 ray_pos = { 0.0f, 10.0f, 0.0f };
    float3 ray_dir = { 0.0f, -1.0f, 0.0f };

    int num_hits;
    int tindex;
    float distance;

    auto print = [&]() {
        printf("    %d hits: index %d, distance %f\n",
            num_hits, tindex, distance);
    };
    printf("RayTrianglesIntersectionTest()\n");

    auto s1_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersection_Generic(ray_pos, ray_dir, vertices.data(),
            indices.data(), num_triangles, tindex, distance);
    }
    auto s1_end = Now();

    print();

    auto s2_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersection_ISPC(ray_pos, ray_dir, vertices.data(),
            indices.data(), num_triangles, tindex, distance);
    }
    auto s2_end = Now();

    print();

    auto s3_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersection_Generic(ray_pos, ray_dir, vertices_flattened.data(), num_triangles, tindex, distance);
    }
    auto s3_end = Now();

    print();

    auto s4_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersection_ISPC(ray_pos, ray_dir, vertices_flattened.data(), num_triangles, tindex, distance);
    }
    auto s4_end = Now();

    print();

    auto s5_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersection_Generic(ray_pos, ray_dir,
            v1x.data(), v1y.data(), v1z.data(),
            v2x.data(), v2y.data(), v2z.data(),
            v3x.data(), v3y.data(), v3z.data(), num_triangles, tindex, distance);
    }
    auto s5_end = Now();

    print();

    auto s6_begin = Now();
    for (int i = 0; i < num_try; ++i) {
        num_hits = RayTrianglesIntersection_ISPC(ray_pos, ray_dir,
            v1x.data(), v1y.data(), v1z.data(),
            v2x.data(), v2y.data(), v2z.data(),
            v3x.data(), v3y.data(), v3z.data(), num_triangles, tindex, distance);
    }
    auto s6_end = Now();

    print();

    printf(
        "    triangle count: %d\n"
        "    ray count: %d\n"
        "    RayTrianglesIntersection (indexed):   Generic %.2fms, ISPC %.2fms\n"
        "    RayTrianglesIntersection (flattened): Generic %.2fms, ISPC %.2fms\n"
        "    RayTrianglesIntersection (SoA):       Generic %.2fms, ISPC %.2fms\n",
        num_triangles,
        num_try,
        float(s1_end - s1_begin) / 1000000.0f,
        float(s2_end - s2_begin) / 1000000.0f,
        float(s3_end - s3_begin) / 1000000.0f,
        float(s4_end - s4_begin) / 1000000.0f,
        float(s5_end - s5_begin) / 1000000.0f,
        float(s6_end - s6_begin) / 1000000.0f);
    printf("\n");
}

int main(int argc, char *argv[])
{
    //Test_Indexed();
    //Test_Sync(false);
    //Test_Get();
    //Test_GenNormals();
    //MatrixSwapHandedness();
    RayTrianglesIntersectionTest();
}
