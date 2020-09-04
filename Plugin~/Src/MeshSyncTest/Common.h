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
    mu::float3 v3n()
    {
        return normalize(mu::float3{ f11(), f11(), f11() });
    }

    // tangent
    mu::float4 v4t()
    {
        mu::float4 r;
        (mu::float3&)r = normalize(mu::float3{ f11(), f11(), f11() });
        r.w = mu::sign(f11());
        return r;
    }
};

