#pragma once

#include "MeshUtils/MeshUtils.h"
using namespace mu;

void GenerateIcoSphereMesh(
    RawVector<int>& counts,
    RawVector<int>& indices,
    RawVector<float3>& points,
    RawVector<float2>& uv,
    float radius,
    int iteration);

void GenerateIcoSphereMesh(
    SharedVector<int>& counts,
    SharedVector<int>& indices,
    SharedVector<float3>& points,
    SharedVector<float2>& uv,
    float radius,
    int iteration);


void GenerateWaveMesh(
    RawVector<int>& counts,
    RawVector<int>& indices,
    RawVector<float3> &points,
    RawVector<float2> &uv,
    float size, float height,
    int resolution,
    float angle,
    bool triangulate = false);

void GenerateWaveMesh(
    SharedVector<int>& counts,
    SharedVector<int>& indices,
    SharedVector<float3> &points,
    SharedVector<float2> &uv,
    float size, float height,
    int resolution,
    float angle,
    bool triangulate = false);

