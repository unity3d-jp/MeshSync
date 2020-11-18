#pragma once

#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSyncConstants.h"


class MeshGenerator {
public:
    static void GenerateIcoSphereMesh(
        RawVector<int>& counts,
        RawVector<int>& indices,
        RawVector<mu::float3>& points,
        RawVector<mu::float2>& uv,
        float radius,
        int iteration);

    static void GenerateIcoSphereMesh(
        SharedVector<int>& counts,
        SharedVector<int>& indices,
        SharedVector<mu::float3>& points,
        SharedVector<mu::float2>& uv,
        float radius,
        int iteration);


    static void GenerateWaveMesh(
        RawVector<int>& counts,
        RawVector<int>& indices,
        RawVector<mu::float3> &points,
        SharedVector<mu::float2> uv[ms::MeshSyncConstants::MAX_UV],
        float size, float height,
        int resolution,
        float angle,
        bool triangulate = false);

    static void GenerateWaveMesh(
        SharedVector<int>& counts,
        SharedVector<int>& indices,
        SharedVector<mu::float3> &points,
        SharedVector<mu::float2> uv[ms::MeshSyncConstants::MAX_UV],
        float size, float height,
        int resolution,
        float angle,
        bool triangulate = false);

};


