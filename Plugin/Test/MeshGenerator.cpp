#include "pch.h"
#include "MeshGenerator.h"

static inline int GetMiddlePoint(int p1, int p2, RawVector<float3>& vertices, std::map<int64_t, int>& cache, float radius)
{
    // first check if we have it already
    bool firstIsSmaller = p1 < p2;
    int64_t smallerIndex = firstIsSmaller ? p1 : p2;
    int64_t greaterIndex = firstIsSmaller ? p2 : p1;
    int64_t key = (smallerIndex << 32) + greaterIndex;

    {
        auto it = cache.find(key);
        if (it != cache.end()) {
            return it->second;
        }
    }

    // not in cache, calculate it
    const auto& point1 = vertices[p1];
    const auto& point2 = vertices[p2];
    float3 middle{
        (point1.x + point2.x) * 0.5f,
        (point1.y + point2.y) * 0.5f,
        (point1.z + point2.z) * 0.5f
    };

    // add vertex makes sure point is on unit sphere
    size_t i = vertices.size();
    vertices.push_back(normalize(middle) * radius);

    // store it, return index
    cache[key] = i;
    return i;
}

void GenerateIcoSphereMesh(
    RawVector<int>& counts,
    RawVector<int>& indices,
    RawVector<float3>& points,
    RawVector<float2>& uv,
    float radius,
    int iteration)
{
    float t = (1.0f + std::sqrt(5.0f)) * 0.5f;

    points = {
        normalize(float3{-1.0f,    t, 0.0f }) * radius,
        normalize(float3{ 1.0f,    t, 0.0f }) * radius,
        normalize(float3{-1.0f,   -t, 0.0f }) * radius,
        normalize(float3{ 1.0f,   -t, 0.0f }) * radius,
        normalize(float3{ 0.0f,-1.0f,    t }) * radius,
        normalize(float3{ 0.0f, 1.0f,    t }) * radius,
        normalize(float3{ 0.0f,-1.0f,   -t }) * radius,
        normalize(float3{ 0.0f, 1.0f,   -t }) * radius,
        normalize(float3{    t, 0.0f,-1.0f }) * radius,
        normalize(float3{    t, 0.0f, 1.0f }) * radius,
        normalize(float3{   -t, 0.0f,-1.0f }) * radius,
        normalize(float3{   -t, 0.0f, 1.0f }) * radius,
    };

    indices = {
        0, 11, 5,
        0, 5, 1,
        0, 1, 7,
        0, 7, 10,
        0, 10, 11,

        1, 5, 9,
        5, 11, 4,
        11, 10, 2,
        10, 7, 6,
        7, 1, 8,

        3, 9, 4,
        3, 4, 2,
        3, 2, 6,
        3, 6, 8,
        3, 8, 9,

        4, 9, 5,
        2, 4, 11,
        6, 2, 10,
        8, 6, 7,
        9, 8, 1,
    };

    std::map<int64_t, int> cache;
    for (int it = 0; it < iteration; it++)
    {
        RawVector<int> indices2;
        size_t n = indices.size();
        for (size_t fi = 0; fi < n; fi += 3)
        {
            int i1 = indices[fi + 0];
            int i2 = indices[fi + 1];
            int i3 = indices[fi + 2];
            int a = GetMiddlePoint(i1, i2, points, cache, radius);
            int b = GetMiddlePoint(i2, i3, points, cache, radius);
            int c = GetMiddlePoint(i3, i1, points, cache, radius);

            int addition[]{
                i1, a, c,
                i2, b, a,
                i3, c, b,
                a, b, c,
            };
            indices2.insert(indices2.end(), std::begin(addition), std::end(addition));
        }
        indices = indices2;
    }

    counts.resize(indices.size() / 3);
    for (int& c : counts) { c = 3; }
}

void GenerateWaveMesh(
    RawVector<int>& counts,
    RawVector<int>& indices,
    RawVector<float3> &points,
    RawVector<float2> &uv,
    float size, float height,
    const int resolution,
    float angle,
    bool triangulate)
{
    const int num_vertices = resolution * resolution;

    // vertices
    points.resize(num_vertices);
    uv.resize(num_vertices);
    for (int iy = 0; iy < resolution; ++iy) {
        for (int ix = 0; ix < resolution; ++ix) {
            int i = resolution*iy + ix;
            float2 pos = {
                float(ix) / float(resolution - 1) - 0.5f,
                float(iy) / float(resolution - 1) - 0.5f
            };
            float d = std::sqrt(pos.x*pos.x + pos.y*pos.y);

            float3& v = points[i];
            v.x = pos.x * size;
            v.y = std::sin(d * 10.0f + angle) * std::max<float>(1.0 - d, 0.0f) * height;
            v.z = pos.y * size;

            float2& t = uv[i];
            t.x = pos.x * 0.5 + 0.5;
            t.y = pos.y * 0.5 + 0.5;
        }
    }

    // topology
    if (triangulate) {
        int num_faces = (resolution - 1) * (resolution - 1) * 2;
        int num_indices = num_faces * 3;

        counts.resize(num_faces);
        indices.resize(num_indices);
        for (int iy = 0; iy < resolution - 1; ++iy) {
            for (int ix = 0; ix < resolution - 1; ++ix) {
                int i = (resolution - 1)*iy + ix;
                counts[i * 2 + 0] = 3;
                counts[i * 2 + 1] = 3;
                indices[i * 6 + 0] = resolution*iy + ix;
                indices[i * 6 + 1] = resolution*(iy + 1) + ix;
                indices[i * 6 + 2] = resolution*(iy + 1) + (ix + 1);
                indices[i * 6 + 3] = resolution*iy + ix;
                indices[i * 6 + 4] = resolution*(iy + 1) + (ix + 1);
                indices[i * 6 + 5] = resolution*iy + (ix + 1);
            }
        }
    }
    else {
        int num_faces = (resolution - 1) * (resolution - 1);
        int num_indices = num_faces * 4;

        counts.resize(num_faces);
        indices.resize(num_indices);
        for (int iy = 0; iy < resolution - 1; ++iy) {
            for (int ix = 0; ix < resolution - 1; ++ix) {
                int i = (resolution - 1)*iy + ix;
                counts[i] = 4;
                indices[i * 4 + 0] = resolution*iy + ix;
                indices[i * 4 + 1] = resolution*(iy + 1) + ix;
                indices[i * 4 + 2] = resolution*(iy + 1) + (ix + 1);
                indices[i * 4 + 3] = resolution*iy + (ix + 1);
            }
        }
    }
}
