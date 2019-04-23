#include "pch.h"
#include "Test.h"
#include "MeshGenerator.h"
#include "Common.h"
using namespace mu;

#ifdef EnableFbxExport
#include "FbxExporter/FbxExporter.h"
#pragma comment(lib, "FbxExporterCore.lib")

void ExportFbxImpl(const char *path,
    const RawVector<int>& indices, int ngon,
    const RawVector<float3>& points, const RawVector<float3>& normals, const RawVector<float4>& tangents,
    const RawVector<float2>& uv, const RawVector<float4>& colors)
{
    fbxe::ExportOptions opt;
    auto ctx = fbxeCreateContext(&opt);
    fbxeCreateScene(ctx, "TestScene");
    auto node = fbxeCreateNode(ctx, nullptr, "Mesh");
    fbxeAddMesh(ctx, node, points.size(),
        (fbxe::float3*)points.data(), (fbxe::float3*)normals.data(), (fbxe::float4*)tangents.data(),
        (fbxe::float2*)uv.data(), (fbxe::float4*)colors.data());
    fbxeAddMeshSubmesh(ctx, node, (fbxe::Topology)(ngon-1), indices.size(), indices.data(), -1);
    fbxeWrite(ctx, path, fbxe::Format::FbxAscii);
    fbxeReleaseContext(ctx);
}
#define ExportFbx(...) ExportFbxImpl(__VA_ARGS__)
#else // EnableFbxExport
#define ExportFbx(...)
#endif // EnableFbxExport


template<class T> struct StrideIterator
{
    T *data;
    size_t stride;
};


TestCase(Test_IndexedArrays)
{
    std::vector<uint16_t> indices16 = { 0,1,0,2,1,3,2 };
    std::vector<uint32_t> indices32 = { 0,1,2,1,2,3 };
    std::vector<float> values = { 0.0f, 10.0f, 20.0f, 30.0f };

    IIArray<uint16_t, float> iia1 = { indices16, values };
    IIArray<uint32_t, float> iia2 = { indices32, values };

    Print("    iia1: ");
    for(auto& v : iia1) { Print("%.f ", v); }
    Print("\n");

    Print("    iia2: ");
    for (auto& v : iia2) { Print("%.f ", v); }
    Print("\n");
}


TestCase(TestMeshRefiner)
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
    RawVector<int> material_ids = {
        0, 1, 2, 3
    };

    RawVector<float2> uv_flattened(indices.size());
    for (int i = 0; i < indices.size(); ++i) {
        uv_flattened[i] = uv[indices[i]];
    }

    RawVector<float3> normals;
    GenerateNormalsWithSmoothAngle(normals, points, counts, indices, 40.0f, false);

    RawVector<float2> uv_refined;
    RawVector<float3> normals_refined;
    RawVector<int> remap_uv, remap_normals;

    mu::MeshRefiner refiner;
    refiner.split_unit = 8;
    refiner.counts = counts;
    refiner.indices = indices;
    refiner.points = points;

    refiner.addExpandedAttribute<float2>(uv_flattened, uv_refined, remap_uv);
    refiner.addExpandedAttribute<float3>(normals, normals_refined, remap_normals);

    refiner.refine();
    refiner.retopology(false);
    refiner.genSubmeshes(material_ids);
}


TestCase(TestNormalsAndTangents)
{
    RawVector<int> indices, counts;
    RawVector<float3> points;
    RawVector<float2> uv;
    GenerateWaveMesh(counts, indices, points, uv, 10.0f, 0.25f, 250, 0.0f, true);

    int num_try = 10;
    int num_points = (int)points.size();
    int num_triangles = (int)indices.size() / 3;
    RawVector<float3> points_f;
    RawVector<float2> uv_f;
    RawVector<float3> normals[6];
    RawVector<float4> tangents[7];
    RawVector<float> psoa[9], usoa[6];


    // generate flattened data
    points_f.resize(indices.size());
    uv_f.resize(indices.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        points_f[i] = points[indices[i]];
        uv_f[i] = uv[indices[i]];
    }

    // generate soa data
    for (auto& v : normals) { v.resize(points.size()); }
    for (auto& v : tangents) { v.resize(points.size()); }
    for (auto& v : psoa) { v.resize(num_triangles); }
    for (auto& v : usoa) { v.resize(num_triangles); }
    for (int ti = 0; ti < num_triangles; ++ti) {
        float3 p[3] = {
            points[indices[ti * 3 + 0]],
            points[indices[ti * 3 + 1]],
            points[indices[ti * 3 + 2]],
        };
        float2 u[3] = {
            uv[indices[ti * 3 + 0]],
            uv[indices[ti * 3 + 1]],
            uv[indices[ti * 3 + 2]],
        };

        for (int i = 0; i < 9; ++i) {
            psoa[i][ti] = ((float*)p)[i];
        }
        for (int i = 0; i < 6; ++i) {
            usoa[i][ti] = ((float*)u)[i];
        }
    }

    auto ValidateNormals = [&](const RawVector<float3>& ns) {
        if (!NearEqual(normals[0].data(), ns.data(), ns.size(), 0.01f)) {
            Print("        *** validation failed ***\n");
        }
    };
    auto ValidateTangents = [&](const RawVector<float4>& ts) {
        if (!NearEqual(tangents[0].data(), ts.data(), ts.size(), 0.01f)) {
            Print("        *** validation failed ***\n");
        }
    };

    Print(
        "    num_vertices: %d\n"
        "    num_triangles: %d\n"
        ,
        (int)points.size(),
        (int)indices.size() / 3);

#define SoAPointsArgs\
    psoa[0].data(), psoa[1].data(), psoa[2].data(),\
    psoa[3].data(), psoa[4].data(), psoa[5].data(),\
    psoa[6].data(), psoa[7].data(), psoa[8].data()

#define SoAUVArgs\
    usoa[0].data(), usoa[1].data(),\
    usoa[2].data(), usoa[3].data(),\
    usoa[4].data(), usoa[5].data()

    // generate normals

    TestScope("GenerateNormals indexed C++", [&]() {
        GenerateNormalsTriangleIndexed_Generic(normals[0].data(), points.data(), indices.data(), num_triangles, num_points);
    }, num_try);

#ifdef muSIMD_GenerateNormalsTriangleIndexed
    TestScope("GenerateNormals indexed ISPC", [&]() {
        GenerateNormalsTriangleIndexed_ISPC(normals[1].data(), points.data(), indices.data(), num_triangles, num_points);
    }, num_try);
    ValidateNormals(normals[1]);
#endif

    TestScope("GenerateNormals flattened C++", [&]() {
        GenerateNormalsTriangleFlattened_Generic(normals[2].data(), points_f.data(), indices.data(), num_triangles, num_points);
    }, num_try);
    ValidateNormals(normals[2]);

#ifdef muSIMD_GenerateNormalsTriangleFlattened
    TestScope("GenerateNormals flattened ISPC", [&]() {
        GenerateNormalsTriangleFlattened_ISPC(normals[3].data(), points_f.data(), indices.data(), num_triangles, num_points);
    }, num_try);
    ValidateNormals(normals[3]);
#endif

    TestScope("GenerateNormals SoA C++", [&]() {
        GenerateNormalsTriangleSoA_Generic(normals[4].data(), SoAPointsArgs, indices.data(), num_triangles, num_points);
    }, num_try);
    ValidateNormals(normals[4]);

#ifdef muSIMD_GenerateNormalsTriangleSoA
    TestScope("GenerateNormals SoA ISPC", [&]() {
        GenerateNormalsTriangleSoA_ISPC(normals[5].data(), SoAPointsArgs, indices.data(), num_triangles, num_points);
    }, num_try);
    ValidateNormals(normals[5]);
#endif


    // generate tangents

    TestScope("GenerateTangents indexed C++", [&]() {
        GenerateTangentsTriangleIndexed_Generic(tangents[0].data(),
            points.data(), uv.data(), normals[0].data(), indices.data(), num_triangles, num_points);
    }, num_try);

#ifdef muSIMD_GenerateTangentsTriangleIndexed
    TestScope("GenerateTangents indexed ISPC", [&]() {
        GenerateTangentsTriangleIndexed_ISPC(tangents[1].data(),
            points.data(), uv.data(), normals[1].data(), indices.data(), num_triangles, num_points);
    }, num_try);
    ValidateTangents(tangents[1]);
#endif

    TestScope("GenerateTangents flattened C++", [&]() {
        GenerateTangentsTriangleFlattened_Generic(tangents[2].data(),
            points_f.data(), uv_f.data(), normals[2].data(), indices.data(), num_triangles, num_points);
    }, num_try);
    ValidateTangents(tangents[2]);

#ifdef muSIMD_GenerateTangentsTriangleFlattened
    TestScope("GenerateTangents flattened ISPC", [&]() {
        GenerateTangentsTriangleFlattened_ISPC(tangents[3].data(),
            points_f.data(), uv_f.data(), normals[3].data(), indices.data(), num_triangles, num_points);
    }, num_try);
    ValidateTangents(tangents[3]);
#endif

    TestScope("GenerateTangents SoA C++", [&]() {
        GenerateTangentsTriangleSoA_Generic(tangents[4].data(),
            SoAPointsArgs, SoAUVArgs, normals[4].data(), indices.data(), num_triangles, num_points);
    }, num_try);
    ValidateTangents(tangents[4]);

#ifdef muSIMD_GenerateTangentsTriangleSoA
    TestScope("GenerateTangents SoA ISPC", [&]() {
        GenerateTangentsTriangleSoA_ISPC(tangents[5].data(),
            SoAPointsArgs, SoAUVArgs, normals[5].data(), indices.data(), num_triangles, num_points);
    }, num_try);
    ValidateTangents(tangents[5]);
#endif

    // try to call CalculateTangents() in Unity.exe
    {
        auto unity_exe = GetModule("Unity.exe");
        if (unity_exe) {
            InitializeSymbols();

            using CalculateTangentsT = void(*)(
                StrideIterator<float3> vertices, StrideIterator<float3> normals, StrideIterator<float2> uv, const int *indices,
                int num_vertices, int num_triangles, StrideIterator<float4> dst);

            auto CalculateTangents = (CalculateTangentsT)FindSymbolByName("?CalculateTangents@@YAXV?$StrideIterator@VVector3f@@@@0V?$StrideIterator@VVector2f@@@@PEBIHHV?$StrideIterator@VVector4f@@@@@Z");
            if (CalculateTangents) {
                StrideIterator<float3> viter = { points.data(), sizeof(float3) };
                StrideIterator<float3> niter = { normals[0].data(), sizeof(float3) };
                StrideIterator<float2> uiter = { uv.data(), sizeof(float2) };
                StrideIterator<float4> titer = { tangents[6].data(), sizeof(float4) };

                TestScope("CalculateTangents (Unity.exe)", [&]() {
                    CalculateTangents(viter, niter, uiter, indices.data(), num_points, num_triangles * 3, titer);
                }, num_try);
                ValidateTangents(tangents[6]);
            }
        }
    }

    //ExportFbx("Wave_IndexedCpp.fbx", indices, 3, points, normals[0], tangents[0], uv, {});
    //ExportFbx("Wave_IndexedISPC.fbx", indices, 3, points, normals[1], tangents[1], uv, {});
    //ExportFbx("Wave_FlattenedCpp.fbx", indices, 3, points, normals[2], tangents[2], uv, {});
    //ExportFbx("Wave_FlattenedISPC.fbx", indices, 3, points, normals[3], tangents[3], uv, {});
    //ExportFbx("Wave_SoACpp.fbx", indices, 3, points, normals[4], tangents[4], uv, {});
    //ExportFbx("Wave_SoAISPC.fbx", indices, 3, points, normals[5], tangents[5], uv, {});
    //if (!tangents[6].empty()) {
    //    ExportFbx("Wave_Unity.fbx", indices, 3, points, normals[0], tangents[6], uv, {});
    //}

#undef SoAUVArgs
#undef SoAPointsArgs
}


TestCase(TestMatrixSwapHandedness)
{
    quatf rot1 = rotate(normalize(float3{0.15f, 0.3f, 0.6f}), 60.0f);
    quatf rot2 = flip_x(rot1);
    float4x4 mat1 = to_mat4x4(rot1);
    float4x4 mat2 = to_mat4x4(rot2);
    float4x4 mat3 = flip_x(mat1);
    float4x4 imat1 = invert(mat1);
    float4x4 imat2 = invert(mat2);
    float4x4 imat3 = flip_x(imat1);

    bool r1 = near_equal(mat2, mat3);
    bool r2 = near_equal(imat2, imat3);
    Print("    %d, %d\n", (int)r1, (int)r2);
}


TestCase(TestMulPoints)
{
    const int num_data = 65536;
    const int num_try = 128;

    RawVector<float3> src, dst1, dst2;
    src.resize(num_data);
    dst1.resize(num_data);
    dst2.resize(num_data);

    float4x4 matrix = transform({ 1.0f, 2.0f, 4.0f }, rotate_y(45.0f), {2.0f, 2.0f, 2.0f});

    for (int i = 0; i < num_data; ++i) {
        src[i] = { (float)i*0.1f, (float)i*0.05f, (float)i*0.025f };
    }

    Print(
        "    num_data: %d\n"
        "    num_try: %d\n",
        num_data,
        num_try);

    TestScope("MulPoints C++", [&]() {
        MulPoints_Generic(matrix, src.data(), dst1.data(), num_data);
    }, num_try);
#ifdef muSIMD_MulPoints3
    TestScope("MulPoints ISPC", [&]() {
        MulPoints_ISPC(matrix, src.data(), dst2.data(), num_data);
    }, num_try);
    if (!NearEqual(dst1.data(), dst2.data(), num_data)) {
        Print("    *** validation failed ***\n");
    }
#endif

    TestScope("MulVectors C++", [&]() {
        MulVectors_Generic(matrix, src.data(), dst1.data(), num_data);
    }, num_try);
#ifdef muSIMD_MulVectors3
    TestScope("MulVectors ISPC", [&]() {
        MulVectors_ISPC(matrix, src.data(), dst2.data(), num_data);
    }, num_try);
    if (!NearEqual(dst1.data(), dst2.data(), num_data)) {
        Print("    *** validation failed ***\n");
    }
#endif
}


TestCase(TestRayTrianglesIntersection)
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

    Print(
        "    triangle count: %d\n"
        "    ray count: %d\n",
        num_triangles,
        num_try);

    auto PrintResult = [&]() {
        Print("        %d hits: index %d, distance %f\n",
            num_hits, tindex, distance);
    };

    TestScope("RayTrianglesIntersection indexed C++", [&]() {
        num_hits = RayTrianglesIntersectionIndexed_Generic(ray_pos, ray_dir, vertices.data(),
            indices.data(), num_triangles, tindex, distance);
    }, num_try);
    PrintResult();

#ifdef muSIMD_RayTrianglesIntersectionIndexed
    TestScope("RayTrianglesIntersection indexed ISPC", [&]() {
        num_hits = RayTrianglesIntersectionIndexed_ISPC(ray_pos, ray_dir, vertices.data(),
            indices.data(), num_triangles, tindex, distance);
    }, num_try);
    PrintResult();
#endif

    TestScope("RayTrianglesIntersection flattened C++", [&]() {
        num_hits = RayTrianglesIntersectionFlattened_Generic(ray_pos, ray_dir, vertices_flattened.data(), num_triangles, tindex, distance);
    }, num_try);
    PrintResult();

#ifdef muSIMD_RayTrianglesIntersectionFlattened
    TestScope("RayTrianglesIntersection flattened ISPC", [&]() {
        num_hits = RayTrianglesIntersectionFlattened_ISPC(ray_pos, ray_dir, vertices_flattened.data(), num_triangles, tindex, distance);
    }, num_try);
    PrintResult();
#endif

    TestScope("RayTrianglesIntersection SoA C++", [&]() {
        num_hits = RayTrianglesIntersectionSoA_Generic(ray_pos, ray_dir,
            v1x.data(), v1y.data(), v1z.data(),
            v2x.data(), v2y.data(), v2z.data(),
            v3x.data(), v3y.data(), v3z.data(), num_triangles, tindex, distance);
    }, num_try);
    PrintResult();

#ifdef muSIMD_RayTrianglesIntersectionSoA
    TestScope("RayTrianglesIntersection SoA ISPC", [&]() {
        num_hits = RayTrianglesIntersectionSoA_ISPC(ray_pos, ray_dir,
            v1x.data(), v1y.data(), v1z.data(),
            v2x.data(), v2y.data(), v2z.data(),
            v3x.data(), v3y.data(), v3z.data(), num_triangles, tindex, distance);
    }, num_try);
    PrintResult();
#endif
}


TestCase(TestPolygonInside)
{
    const int num_try = 100;
    const int ngon = 80000;
    RawVector<float2> poly;
    RawVector<float> polyx, polyy;

    poly.resize(ngon); polyx.resize(ngon); polyy.resize(ngon);
    for (int i = 0; i < ngon; ++i) {
        float a = (360.0f / ngon) * i * DegToRad;
        poly[i] = { std::sin(a), std::cos(a) };
        polyx[i] = poly[i].x;
        polyy[i] = poly[i].y;
    }


    float2 points[]{
        float2{ 0.0f, 0.0f },
        float2{ 0.1f, 0.1f },
        float2{ 0.2f, 0.2f },
        float2{ 0.3f, 0.3f },
        float2{ 0.4f, 0.4f },
        float2{ 0.5f, 0.5f },
    };
    float2 pmin, pmax;
    int num_inside = 0;

    auto PrintResult = [&]() {
        Print("        num_inside: %d\n", num_inside);
    };


    num_inside = 0;
    TestScope("PolyInside C++", [&]() {
        MinMax_Generic(poly.data(), ngon, pmin, pmax);
        for (int pi = 0; pi < countof(points); ++pi) {
            if (PolyInside_Generic(poly.data(), ngon, pmin, pmax, points[pi])) {
                ++num_inside;
            }
        }
    }, num_try);
    PrintResult();

#ifdef muSIMD_PolyInside
    num_inside = 0;
    TestScope("PolyInside ISPC", [&]() {
        MinMax_ISPC(poly.data(), ngon, pmin, pmax);
        for (int pi = 0; pi < countof(points); ++pi) {
            if (PolyInside_ISPC(poly.data(), ngon, pmin, pmax, points[pi])) {
                ++num_inside;
            }
        }
    }, num_try);
    PrintResult();
#endif

    num_inside = 0;
    TestScope("PolyInside SoA C++", [&]() {
        MinMax_Generic(poly.data(), ngon, pmin, pmax);
        for (int pi = 0; pi < countof(points); ++pi) {
            if (PolyInside_Generic(polyx.data(), polyy.data(), ngon, pmin, pmax, points[pi])) {
                ++num_inside;
            }
        }
    }, num_try);
    PrintResult();


#ifdef muSIMD_PolyInsideSoA
    num_inside = 0;
    TestScope("PolyInside SoA ISPC", [&]() {
        MinMax_ISPC(poly.data(), ngon, pmin, pmax);
        for (int pi = 0; pi < countof(points); ++pi) {
            if (PolyInside_ISPC(polyx.data(), polyy.data(), ngon, pmin, pmax, points[pi])) {
                ++num_inside;
            }
        }
    }, num_try);
    PrintResult();
#endif
}


TestCase(TestEdge)
{
    {
        RawVector<float3> points = {
            { 0.0f, 0.5f, 0.0f },
            { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f },
            { -1.0f, 0.0f, 0.0f },
        };
        RawVector<int> indices = {
            0, 1, 2,
            0, 2, 3,
            0, 3, 1,
        };

        MeshConnectionInfo connection;
        connection.buildConnection(indices, 3, points);

        for (int vi = 0; vi < 4; ++vi) {
            bool is_edge = OnEdge(indices, 3, points, connection, vi);
            Print("    IsEdge(): %d %d\n", vi, (int)is_edge);
        }

        RawVector<int> edges;
        RawVector<int> vi = { 1 };
        SelectEdge(indices, 3, points, vi, [&](int vi) { edges.push_back(vi); });

        Print("    SelectEdge (triangles):");
        for (int e : edges) {
            Print(" %d", e);
        }
        Print("\n");
    }
    Print("\n");

    {
        RawVector<float3> points(4 * 4);
        RawVector<int> indices(3 * 3 * 4);

        for (int yi = 0; yi < 4; ++yi) {
            for (int xi = 0; xi < 4; ++xi) {
                points[4 * yi + xi] = {(float)xi, 0.0f, (float)yi};
            }
        }
        for (int yi = 0; yi < 3; ++yi) {
            for (int xi = 0; xi < 3; ++xi) {
                int i = 3 * yi + xi;
                indices[4 * i + 0] = 4 * (yi + 0) + (xi + 0);
                indices[4 * i + 1] = 4 * (yi + 0) + (xi + 1);
                indices[4 * i + 2] = 4 * (yi + 1) + (xi + 1);
                indices[4 * i + 3] = 4 * (yi + 1) + (xi + 0);
            }
        }
        indices.erase(indices.begin() + 4 * 4, indices.begin() + 5 * 4);

        RawVector<int> counts = { 4,4,4,4,4,4,4,4 };
        RawVector<int> offsets = { 0,4,8,12,16,20,24,28 };
        for (int i = 0; i < 8; ++i) {
            counts[i] = 4;
            offsets[i] = i * 4;
        }

        MeshConnectionInfo connection;
        connection.buildConnection(indices, counts, points);

        for (int vi = 0; vi < 16; ++vi) {
            bool is_edge = OnEdge(indices, 4, points, connection, vi);
            Print("    IsEdge(): %d %d\n", vi, (int)is_edge);
        }

        RawVector<int> edges;
        RawVector<int> vi = { 1 };

        SelectEdge(indices, counts, offsets, points, vi, [&](int vi) { edges.push_back(vi); });
        Print("    SelectEdge (quads):");
        for (int e : edges) {
            Print(" %d", e);
        }
        Print("\n");

        edges.clear();
        vi[0] = 5;
        SelectEdge(indices, counts, offsets, points, vi, [&](int vi) { edges.push_back(vi); });
        Print("    SelectEdge (quads):");
        for (int e : edges) {
            Print(" %d", e);
        }
        Print("\n");
    }
}

TestCase(TestHandedness)
{
    {
        quatf rot1 = rotate(normalize(float3{ 0.2f, 0.5f, 1.0f }), 30.0f);
        quatf rot2 = rotate(normalize(float3{ 0.2f, 1.0f, 0.5f }), 30.0f);
        Print("ok");
    }
    {
        float4 xdir{ 1.0f, 0.0f, 0.0f, 0.0f };
        float4 ydir{ 0.0f, 1.0f, 0.0f, 0.0f };
        float4 zdir{ 0.0f, 0.0f, 1.0f, 0.0f };

        quatf rot1 = rotate_y(90.0f * DegToRad);
        quatf rot2 = swap_yz(rot1);

        float4
            x1 = to_mat4x4(rot1) * xdir,
            x2 = to_mat4x4(rot2) * xdir,
            y1 = to_mat4x4(rot1) * ydir,
            y2 = to_mat4x4(rot2) * ydir,
            z2 = to_mat4x4(rot2) * zdir,
            z1 = to_mat4x4(rot1) * zdir;
        Print("ok");
    }
}

TestCase(TestMatrixExtraction)
{
    // parent
    auto pos1 = float3{ 1.0f, 2.0f, 3.0f };
    auto rot1 = rotate_xyz(float3{ 15.0f * DegToRad, 30.0f * DegToRad, 60.0f * DegToRad });
    auto scl1 = float3{ 1.0f, -0.5f, 0.25f };

    // child
    auto pos2 = float3{ -5.0f, -2.5f, -1.0f };
    auto rot2 = rotate_xyz(float3{ -90.0f * DegToRad, -60.0f * DegToRad, -30.0f * DegToRad });
    auto scl2 = float3{ -3.0f, -1.0f, 2.0f };

    auto mat_parent = transform(pos1, rot1, scl1);
    auto mat_invert_parent = invert(mat_parent);
    auto mat_basis = transform(pos2, rot2, scl2);

    auto mat_local = mat_invert_parent * mat_basis;

    auto epos = extract_position(mat_local);
    auto erot = extract_rotation(mat_local);
    auto escl = extract_scale(mat_local);

    auto mat_tmp = mat_local;
    mat_tmp *= invert(translate(epos));
    mat_tmp *= invert(to_mat4x4(rot2));
    auto scl = mul_v(mat_invert_parent, scl2);

    Print("ok");
}

TestCase(TestSum)
{
    const size_t input_size = 10000000;

    RawVector<float> input(input_size);
    for (int i = 0; i < input_size; ++i)
        input[i] = (float)i;

    TestScope("SumInt32_Generic", [&]() {
        auto sum = SumInt32_Generic((uint32_t*)input.data(), input.size());
        Print("sum: %llu\n", sum);
    }, 1);
    TestScope("SumInt32_ISPC", [&]() {
        auto sum = SumInt32_ISPC((uint32_t*)input.data(), input.size());
        Print("sum: %llu\n", sum);
    }, 1);
    TestScope("SumInt32", [&]() {
        auto sum = SumInt32(input.data(), sizeof(float) * input.size());
        Print("sum: %llu\n", sum);
    }, 1);
}

TestCase(TestCompareRawVector)
{
    const size_t input_size = 10000000;

    RawVector<float> input1(input_size);
    RawVector<float> input2(input_size);
    RawVector<float> input3(input_size);
    input1.resize(input_size);
    for (int i = 0; i < input_size; ++i) {
        input1[i] = (float)i;
        input2[i] = (float)i;
        input3[i] = (float)i * 1.1f;
    }

    TestScope("compare12", [&]() {
        auto result = input1 == input2;
        Print("result: %d\n", result);
    }, 1);
    TestScope("compare13", [&]() {
        auto result = input1 == input3;
        Print("result: %d\n", result);
    }, 1);
}


template<class T, size_t N>
void FloatIntConversionImpl(float(&src)[N], T (&tmp)[N], float(&dst)[N])
{
    for (int i = 0; i < N; ++i) {
        tmp[i] = src[i];
        dst[i] = tmp[i];
    }
}

TestCase(Test_Norm)
{
    {
        const int N = 9;
        float data[N] = { 0.0f, 0.1f, 0.5f, 1.0f, 5.0f, -0.1f, -0.5f, -1.0f, -5.0f };

        float expected_signed[N];
        float expected_unsigned[N];
        for (int i = 0; i < N; ++i) {
            expected_signed[i] = clamp11(data[i]);
            expected_unsigned[i] = clamp01(data[i]);
        }

        snorm8 ts8[N]; float dst_s8[N];
        FloatIntConversionImpl(data, ts8, dst_s8);
        Expect(NearEqual(dst_s8, expected_signed, N, 1e-2f));

        unorm8 tu8[N]; float dst_u8[N];
        FloatIntConversionImpl(data, tu8, dst_u8);
        Expect(NearEqual(dst_u8, expected_unsigned, N, 1e-2f));

        unorm8n tu8n[N]; float dst_u8n[N];
        FloatIntConversionImpl(data, tu8n, dst_u8n);
        Expect(NearEqual(dst_u8n, expected_signed, N, 1e-2f));

        snorm16 ts16[N]; float dst_s16[N];
        FloatIntConversionImpl(data, ts16, dst_s16);
        Expect(NearEqual(dst_s16, expected_signed, N));

        unorm16 tu16[N]; float dst_u16[N];
        FloatIntConversionImpl(data, tu16, dst_u16);
        Expect(NearEqual(dst_u16, expected_unsigned, N));

        snorm24 ts24[N]; float dst_s24[N];
        FloatIntConversionImpl(data, ts24, dst_s24);
        Expect(NearEqual(dst_s24, expected_signed, N));

        snorm32 ts32[N]; float dst_s32[N];
        FloatIntConversionImpl(data, ts32, dst_s32);
        Expect(NearEqual(dst_s32, expected_signed, N));
    }
#ifdef muSIMD_Float_Norm_Conversion
    {
        const int N = 1000000;
        const int T = 5;
        RawVector<float> data(N);

        float step = 3.0f / N;
        for (int i = 0; i < N; ++i) {
            data[i] = -1.5f + step * i;
        }

        RawVector<float> expected_signed(N);
        RawVector<float> expected_unsigned(N);
        for (int i = 0; i < N; ++i) {
            expected_signed[i] = clamp11(data[i]);
            expected_unsigned[i] = clamp01(data[i]);
        }

        {
            RawVector<snorm8> ts8(N); RawVector<float> dst_s8(N);
            TestScope("F32ToS8_ISPC", [&]() {
                F32ToS8_ISPC(ts8.data(), data.data(), N);
                S8ToF32_ISPC(dst_s8.data(), ts8.data(), N);
            }, T);
            Expect(NearEqual(dst_s8.data(), expected_signed.data(), N, 1e-2f));
            TestScope("F32ToS8_Generic", [&]() {
                F32ToS8_Generic(ts8.data(), data.data(), N);
                S8ToF32_Generic(dst_s8.data(), ts8.data(), N);
            }, T);
            Expect(NearEqual(dst_s8.data(), expected_signed.data(), N, 1e-2f));
        }
        {
            RawVector<unorm8> tu8(N); RawVector<float> dst_u8(N);
            TestScope("F32ToU8_ISPC", [&]() {
                F32ToU8_ISPC(tu8.data(), data.data(), N);
                U8ToF32_ISPC(dst_u8.data(), tu8.data(), N);
            }, T);
            Expect(NearEqual(dst_u8.data(), expected_unsigned.data(), N, 1e-2f));
            TestScope("F32ToU8_Generic", [&]() {
                F32ToU8_Generic(tu8.data(), data.data(), N);
                U8ToF32_Generic(dst_u8.data(), tu8.data(), N);
            }, T);
            Expect(NearEqual(dst_u8.data(), expected_unsigned.data(), N, 1e-2f));
        }
        {
            RawVector<unorm8n> tu8n(N); RawVector<float> dst_u8n(N);
            TestScope("F32ToU8N_ISPC", [&]() {
                F32ToU8N_ISPC(tu8n.data(), data.data(), N);
                U8NToF32_ISPC(dst_u8n.data(), tu8n.data(), N);
            }, T);
            Expect(NearEqual(dst_u8n.data(), expected_signed.data(), N, 1e-2f));
            TestScope("F32ToU8N_Generic", [&]() {
                F32ToU8N_Generic(tu8n.data(), data.data(), N);
                U8NToF32_Generic(dst_u8n.data(), tu8n.data(), N);
            }, T);
            Expect(NearEqual(dst_u8n.data(), expected_signed.data(), N, 1e-2f));
        }
        {
            RawVector<snorm16> ts16(N); RawVector<float> dst_s16(N);
            TestScope("F32ToS16_ISPC", [&]() {
                F32ToS16_ISPC(ts16.data(), data.data(), N);
                S16ToF32_ISPC(dst_s16.data(), ts16.data(), N);
            }, T);
            Expect(NearEqual(dst_s16.data(), expected_signed.data(), N));
            TestScope("F32ToS16_Generic", [&]() {
                F32ToS16_Generic(ts16.data(), data.data(), N);
                S16ToF32_Generic(dst_s16.data(), ts16.data(), N);
            }, T);
            Expect(NearEqual(dst_s16.data(), expected_signed.data(), N));
        }
        {
            RawVector<unorm16> tu16(N); RawVector<float> dst_u16(N);
            TestScope("F32ToU16_ISPC", [&]() {
                F32ToU16_ISPC(tu16.data(), data.data(), N);
                U16ToF32_ISPC(dst_u16.data(), tu16.data(), N);
            }, T);
            Expect(NearEqual(dst_u16.data(), expected_unsigned.data(), N));
            TestScope("F32ToU16_Generic", [&]() {
                F32ToU16_Generic(tu16.data(), data.data(), N);
                U16ToF32_Generic(dst_u16.data(), tu16.data(), N);
            }, T);
            Expect(NearEqual(dst_u16.data(), expected_unsigned.data(), N));
        }
        {
            RawVector<snorm24> ts24(N); RawVector<float> dst_s24(N);
            TestScope("F32ToS24_ISPC", [&]() {
                F32ToS24_ISPC(ts24.data(), data.data(), N);
                S24ToF32_ISPC(dst_s24.data(), ts24.data(), N);
            }, T);
            Expect(NearEqual(dst_s24.data(), expected_signed.data(), N));
            TestScope("F32ToS24_Generic", [&]() {
                F32ToS24_Generic(ts24.data(), data.data(), N);
                S24ToF32_Generic(dst_s24.data(), ts24.data(), N);
            }, T);
            Expect(NearEqual(dst_s24.data(), expected_signed.data(), N));
        }
        {
            RawVector<snorm32> ts32(N); RawVector<float> dst_s32(N);
            TestScope("F32ToS32_ISPC", [&]() {
                F32ToS32_ISPC(ts32.data(), data.data(), N);
                S32ToF32_ISPC(dst_s32.data(), ts32.data(), N);
            }, T);
            Expect(NearEqual(dst_s32.data(), expected_signed.data(), N));
            TestScope("F32ToS32_Generic", [&]() {
                F32ToS32_Generic(ts32.data(), data.data(), N);
                S32ToF32_Generic(dst_s32.data(), ts32.data(), N);
            }, T);
            Expect(NearEqual(dst_s32.data(), expected_signed.data(), N));
        }
    }
#endif
}

TestCase(Test_Quat32)
{
    const int N = 100;
    const float eps = 0.01f;

    RawVector<quath> qf16(N); RawVector<quat32> r16(N);
    RawVector<quatf> qf32(N); RawVector<quat32> r32(N);
    RawVector<quatd> qf64(N); RawVector<quat32> r64(N);

    float3 forward = { 0.0f, 0.0f, 1.0f};
    Random rnd;
    for (int i = 0; i < N; ++i) {
        auto axis = rnd.v3n();
        auto angle = rnd.f11() * mu::PI;

        qf16[i] = to<quath>(rotate(axis, angle));
        r16[i] = qf16[i];
        auto hfa = apply_rotation(to<quatf>(qf16[i]), forward);
        auto hfb = apply_rotation(to<quatf>(r16[i]), forward);
        Expect(near_equal(hfa, hfb, eps));

        qf32[i] = rotate(axis, angle);
        r32[i] = qf32[i];
        auto ffa = apply_rotation(qf32[i], forward);
        auto ffb = apply_rotation(to<quatf>(r32[i]), forward);
        Expect(near_equal(ffa, ffb, eps));

        qf64[i] = to<quatd>(rotate(axis, angle));
        r64[i] = qf64[i];
        auto dfa = apply_rotation(to<quatf>(qf64[i]), forward);
        auto dfb = apply_rotation(to<quatf>(r64[i]), forward);
        Expect(near_equal(dfa, dfb, eps));
    }
}

TestCase(Test_S10x3)
{
    const int N = 100;
    const float eps = 0.01f;

    Random rnd;
    for (int i = 0; i < N; ++i) {
        float4 tangent;
        (float3&)tangent = rnd.v3n();
        tangent.w = i % 2 == 0 ? 1.0f : -1.0f;

        snorm10x3 encoded = mu::encode_tangent(tangent);
        float4 decoded = mu::decode_tangent(encoded);
        Expect(near_equal(tangent, decoded, eps));
    }
}

TestCase(Test_BoundedArray)
{
    const int N = 100;
    const float eps = 0.01f;

    Random rnd;

    RawVector<int> data1i(N), tmp1i(N);
    RawVector<float> data1(N), tmp1(N);
    RawVector<float2> data2(N), tmp2(N);
    RawVector<float3> data3(N), tmp3(N);
    RawVector<float4> data4(N), tmp4(N);
    RawVector<float4> data_tangents(N), tmp_tangents(N);
    for (int i = 0; i < N; ++i) {
        data1i[i] = (rnd.f01() * 60000.0f) + 10000;

        data1[i] = rnd.f11() * 1.0f;
        data2[i] = { rnd.f01()*2.0f, rnd.f01()*2.0f - 2.0f };
        data3[i] = rnd.v3n();
        data4[i] = { rnd.f01() + 5.0f, rnd.f01() + 2.0f, rnd.f01() - 2.0f, rnd.f01() - 5.0f };

        float4 tangent;
        (float3&)tangent = rnd.v3n();
        tangent.w = i % 2 == 0 ? 1.0f : -1.0f;
        data_tangents[i] = tangent;
    }

    BoundedArrayU16I ba1i_16;
    BoundedArrayU8 ba1_8;
    BoundedArrayU8x2 ba2_8;
    BoundedArrayU10x3 ba3_10;
    BoundedArrayU16x3 ba3_16;
    BoundedArrayU16x4 ba4_16;
    PackedArrayS10x3 batan;

    encode(ba1i_16, data1i);
    decode(tmp1i, ba1i_16);
    Expect(data1i == tmp1i);

    encode(ba1_8, data1);
    decode(tmp1, ba1_8);
    Expect(NearEqual(data1.data(), tmp1.data(), N, eps));

    encode(ba2_8, data2);
    decode(tmp2, ba2_8);
    Expect(NearEqual(data2.data(), tmp2.data(), N, eps));

    encode(ba3_10, data3);
    decode(tmp3, ba3_10);
    Expect(NearEqual_Generic((float*)data3.data(), (float*)tmp3.data(), N*3, eps));

    encode(ba3_16, data3);
    decode(tmp3, ba3_16);
    Expect(NearEqual(data3.data(), tmp3.data(), N, eps));

    encode(ba4_16, data4);
    decode(tmp4, ba4_16);
    Expect(NearEqual(data4.data(), tmp4.data(), N, eps));

    encode_tangents(batan, data_tangents);
    decode_tangents(tmp_tangents, batan);
    Expect(NearEqual(data_tangents.data(), tmp_tangents.data(), N, eps));
}

TestCase(Test_RemoveNamespace)
{
    auto remove_namespace = [](std::string path) {
        static const std::regex s_remove_head("^([^/]+:)");
        static const std::regex s_remove_leaf("/([^/]+:)");

        auto ret = std::regex_replace(path, s_remove_head, "");
        return std::regex_replace(ret, s_remove_leaf, "/");
    };

    Expect(remove_namespace("name") == "name");
    Expect(remove_namespace("ns1::name") == "name");
    Expect(remove_namespace("ns1::ns2::ns3::name") == "name");
    Expect(remove_namespace("/parent/child") == "/parent/child");
    Expect(remove_namespace("/ns1::parent/ns1::child") == "/parent/child");
    Expect(remove_namespace("/ns1::ns2::ns3::parent/ns1::ns2::ns3::child") == "/parent/child");
}


TestCase(Test_UniqueUnsorted)
{
    int input[] = { 5,1,3,6,2,4,3,4,5,4,6,7,6 };
    // std::size() require C++17
    size_t len = std::distance(std::begin(input), std::end(input));
    auto pos = unique_unsorted(input, input + len);
    auto n = std::distance(input, pos);
    Expect(n == 7);

    for (size_t i = 0; i < n; ++i)
        Print("%d ", input[i]);
    Print("\n");
}


TestCase(Test_Quadify)
{
    {
        float3 points[] = {
            {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {2.0f, 1.0f, 0.0f},
            {0.0f, 2.0f, 0.0f}, {1.0f, 2.0f, 0.0f}, {2.0f, 2.0f, 0.0f},
        };
        int triangles[] = {
            0,1,4, 0,4,3, 1,2,5, 1,5,4,
            3,4,7, 3,7,6, 4,5,8, 4,8,7,
        };

        RawVector<int> dst_indices, dst_counts;
        QuadifyTriangles(points, triangles, false, 15.0f, dst_indices, dst_counts);
        Expect(dst_counts.size() == 4);
    }
}
