#pragma once

struct Random
{
    std::mt19937 r;
    std::uniform_real_distribution<float> d01, d11;

    Random()
    {
        r.seed(0 /*std::random_device()()*/);
        d01 = std::uniform_real_distribution<float>(0.0f, 1.0f);
        d11 = std::uniform_real_distribution<float>(-1.0f, 1.0f);
    }

    float f01()
    {
        return d01(r);
    }

    float f11()
    {
        return d11(r);
    }

    // normalized vector
    float3 v3n()
    {
        return normalize(float3{ f11(), f11(), f11() });
    }

    // tangent
    float4 v4t()
    {
        float4 r;
        (float3&)r = normalize(float3{ f11(), f11(), f11() });
        r.w = sign(f11());
        return r;
    }
};

