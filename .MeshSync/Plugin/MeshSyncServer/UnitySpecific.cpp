#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncServer.h"

using namespace mu;

struct Keyframe
{
    float time;
    float value;
    float in_tangent;
    float out_tangent;
    int tangent_mode; // #ifdef UNITY_EDITOR
    int weighted_mode;
    float in_weight;
    float out_weight;
};

enum InterpolationMode
{
    Smooth,
    Linear,
    Constant,
};
enum TangentMode
{
    kTangentModeFree = 0,
    kTangentModeAuto = 1,
    kTangentModeLinear = 2,
    kTangentModeConstant = 3,
    kTangentModeClampedAuto = 4,
};
const int kBrokenMask = 1 << 0;
const int kLeftTangentMask = 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4;
const int kRightTangentMask = 1 << 5 | 1 << 6 | 1 << 7 | 1 << 8;
const float kTimeEpsilon = 0.00001f;
const float kDefaultWeight = 1.0f / 3.0f;
const float kCurveTimeEpsilon = 0.00001f;

static inline float LinearTangent(IArray<Keyframe>& curve, int i1, int i2)
{
    const auto& k1 = curve[i1];
    const auto& k2 = curve[i2];

    float dt = k2.time - k1.time;
    if (std::abs(dt) < kTimeEpsilon)
        return 0.0f;

    return (k2.value - k1.value) / dt;
}

static inline TangentMode GetLeftTangentMode(const Keyframe& key)
{
    return static_cast<TangentMode>((key.tangent_mode & kLeftTangentMask) >> 1);
}

static inline TangentMode GetRightTangentMode(const Keyframe& key)
{
    return static_cast<TangentMode>((key.tangent_mode & kRightTangentMask) >> 5);
}

static inline float SafeDiv(float y, float x)
{
    if (std::abs(x) > kCurveTimeEpsilon)
        return y / x;
    else
        return 0;
}

static inline void SmoothTangents(IArray<Keyframe>& curve, int index, float bias)
{
    if (curve.size() < 2)
        return;

    auto& key = curve[index];
    if (index == 0) {
        key.in_tangent = key.out_tangent = 0;

        if (curve.size() > 1)
            key.in_weight = key.out_weight = kDefaultWeight;
    }
    else if (index == curve.size() - 1) {
        key.in_tangent = key.out_tangent = 0;

        if (curve.size() > 1)
            key.in_weight = key.out_weight = kDefaultWeight;
    }
    else {
        float dx1 = key.time - curve[index - 1].time;
        float dy1 = key.value - curve[index - 1].value;

        float dx2 = curve[index + 1].time - key.time;
        float dy2 = curve[index + 1].value - key.value;

        float dx = dx1 + dx2;
        float dy = dy1 + dy2;

        float m1 = SafeDiv(dy1, dx1);
        float m2 = SafeDiv(dy2, dx2);

        float m = SafeDiv(dy, dx);

        if ((m1 > 0 && m2 > 0) || (m1 < 0 && m2 < 0)) {
            float lower_bias = (1.0f - bias) * 0.5f;
            float upper_bias = lower_bias + bias;

            float lower_dy = dy * lower_bias;
            float upper_dy = dy * upper_bias;

            if (std::abs(dy1) >= std::abs(upper_dy))
            {
                float b = SafeDiv(dy1 - upper_dy, lower_dy);
                //T mp = b * m2 + (1.0F - b) * m;
                float mp = (1.0F - b) * m;

                key.in_tangent = mp; key.out_tangent = mp;
            }
            else if (std::abs(dy1) < std::abs(lower_dy)) {
                float b = SafeDiv(dy1, lower_dy);
                //T mp = b * m + (1.0F - b) * m1;
                float mp = b * m;

                key.in_tangent = mp; key.out_tangent = mp;
            }
            else {
                key.in_tangent = m; key.out_tangent = m;
            }
        }
        else {
            key.in_tangent = key.out_tangent = 0.0f;
        }

        key.in_weight = kDefaultWeight;
        key.out_weight = kDefaultWeight;
    }
}

static inline void UpdateTangents(IArray<Keyframe>& curve, int index)
{
    auto& key = curve[index];

    // linear
    if (GetLeftTangentMode(key) == kTangentModeLinear && index >= 1)
        key.in_tangent = LinearTangent(curve, index, index - 1);
    if (GetRightTangentMode(key) == kTangentModeLinear && index + 1 < curve.size())
        key.out_tangent = LinearTangent(curve, index, index + 1);

    // smooth
    if (GetLeftTangentMode(key) == kTangentModeClampedAuto || GetRightTangentMode(key) == kTangentModeClampedAuto)
        SmoothTangents(curve, index, 0.5f);

    // constant
    if (GetLeftTangentMode(key) == kTangentModeConstant)
        key.in_tangent = std::numeric_limits<float>::infinity();
    if (GetRightTangentMode(key) == kTangentModeConstant)
        key.out_tangent = std::numeric_limits<float>::infinity();
}

static void SetTangentMode(Keyframe *key, int n, InterpolationMode im)
{
    TangentMode tangent_mode;
    switch (im) {
    case Linear: tangent_mode = kTangentModeLinear; break;
    case Constant: tangent_mode = kTangentModeConstant; break;
    default: tangent_mode = kTangentModeClampedAuto; break;
    }

    for (int i = 0; i < n; ++i) {
        auto& k = key[i];
        k.tangent_mode |= kBrokenMask;

        k.tangent_mode &= ~kLeftTangentMask;
        k.tangent_mode |= (int)tangent_mode << 1;

        k.tangent_mode &= ~kRightTangentMask;
        k.tangent_mode |= (int)tangent_mode << 5;

        k.in_weight = k.out_weight = kDefaultWeight;
    }

    IArray<Keyframe> curve{ key, (size_t)n };
    for (int i = 0; i < n; ++i)
        UpdateTangents(curve, i);
}

static inline void FillCurve(const RawVector<ms::TVP<bool>>& src, Keyframe *x, InterpolationMode it)
{
    int n = (int)src.size();
    for (int i = 0; i < n; ++i) {
        const auto v = src[i];
        x[i].time = v.time;
        x[i].value = (float)v.value;
    }
    SetTangentMode(x, n, it);
}
static inline void FillCurve(const RawVector<ms::TVP<float>>& src, Keyframe *x, InterpolationMode it)
{
    int n = (int)src.size();
    for (int i = 0; i < n; ++i) {
        const auto v = src[i];
        x[i].time = v.time;
        x[i].value = v.value;
    }
    SetTangentMode(x, n, it);
}
static inline void FillCurves(const RawVector<ms::TVP<float3>>& src, Keyframe *x, Keyframe *y, Keyframe *z, InterpolationMode it)
{
    int n = (int)src.size();
    for (int i = 0; i < n; ++i) {
        const auto v = src[i];
        x[i].time = v.time;
        x[i].value = v.value.x;
        y[i].time = v.time;
        y[i].value = v.value.y;
        z[i].time = v.time;
        z[i].value = v.value.z;
    }
    SetTangentMode(x, n, it);
    SetTangentMode(y, n, it);
    SetTangentMode(z, n, it);
}
static inline void FillCurves(const RawVector<ms::TVP<float4>>& src, Keyframe *x, Keyframe *y, Keyframe *z, Keyframe *w, InterpolationMode it)
{
    int n = (int)src.size();
    for (int i = 0; i < n; ++i) {
        const auto v = src[i];
        x[i].time = v.time;
        x[i].value = v.value.x;
        y[i].time = v.time;
        y[i].value = v.value.y;
        z[i].time = v.time;
        z[i].value = v.value.z;
        w[i].time = v.time;
        w[i].value = v.value.w;
    }
    SetTangentMode(x, n, it);
    SetTangentMode(y, n, it);
    SetTangentMode(z, n, it);
    SetTangentMode(w, n, it);
}
static inline void FillCurves(const RawVector<ms::TVP<quatf>>& src, Keyframe *x, Keyframe *y, Keyframe *z, Keyframe *w, InterpolationMode it)
{
    int n = (int)src.size();
    for (int i = 0; i < n; ++i) {
        const auto v = src[i];
        x[i].time = v.time;
        x[i].value = v.value.x;
        y[i].time = v.time;
        y[i].value = v.value.y;
        z[i].time = v.time;
        z[i].value = v.value.z;
        w[i].time = v.time;
        w[i].value = v.value.w;
    }
    SetTangentMode(x, n, it);
    SetTangentMode(y, n, it);
    SetTangentMode(z, n, it);
    SetTangentMode(w, n, it);
}
static inline void FillCurvesEuler(const RawVector<ms::TVP<quatf>>& src, Keyframe *x, Keyframe *y, Keyframe *z, InterpolationMode it)
{
    int n = (int)src.size();
    float3 prev;
    for (int i = 0; i < n; ++i) {
        const auto v = src[i];
        auto r = mu::to_eulerZXY(v.value) * mu::Rad2Deg;
        if (i > 0) {
            // make continuous
            auto d = r - mu::mod(prev, 360.0f);
            auto x = mu::abs(d);
            auto x1 = mu::abs(d - 360.0f);
            auto x2 = mu::abs(d + 360.0f);

            if (x1.x < x.x)
                r.x -= 360.0f;
            else if (x2.x < x.x)
                r.x += 360.0f;

            if (x1.y < x.y)
                r.y -= 360.0f;
            else if (x2.y < x.y)
                r.y += 360.0f;

            if (x1.z < x.z)
                r.z -= 360.0f;
            else if (x2.z < x.z)
                r.z += 360.0f;

            auto c = prev / 360.0f;
            c -= mu::frac(c);
            r += c * 360.0f;
        }
        prev = r;

        x[i].time = v.time;
        x[i].value = r.x;
        y[i].time = v.time;
        y[i].value = r.y;
        z[i].time = v.time;
        z[i].value = r.z;
    }
    SetTangentMode(x, n, it);
    SetTangentMode(y, n, it);
    SetTangentMode(z, n, it);
}

msAPI void msTransformAFillTranslation(ms::TransformAnimation *self, Keyframe *x, Keyframe *y, Keyframe *z, InterpolationMode it)
{
    FillCurves(self->translation, x, y, z, it);
}
msAPI void msTransformAFillRotation(ms::TransformAnimation *self, Keyframe *x, Keyframe *y, Keyframe *z, Keyframe *w, InterpolationMode it)
{
    FillCurves(self->rotation, x, y, z, w, it);
}
msAPI void msTransformAFillRotationEuler(ms::TransformAnimation *self, Keyframe *x, Keyframe *y, Keyframe *z, InterpolationMode it)
{
    FillCurvesEuler(self->rotation, x, y, z, it);
}
msAPI void msTransformAFillScale(ms::TransformAnimation *self, Keyframe *x, Keyframe *y, Keyframe *z, InterpolationMode it)
{
    FillCurves(self->scale, x, y, z, it);
}
msAPI void msTransformAFillVisible(ms::TransformAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->visible, v, it);
}

msAPI void msCameraAFillFov(ms::CameraAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->fov, v, it);
}
msAPI void msCameraAFillNear(ms::CameraAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->near_plane, v, it);
}
msAPI void msCameraAFillFar(ms::CameraAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->far_plane, v, it);
}
msAPI void msCameraAFillHAperture(ms::CameraAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->horizontal_aperture, v, it);
}
msAPI void msCameraAFillVAperture(ms::CameraAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->vertical_aperture, v, it);
}
msAPI void msCameraAFillFocalLength(ms::CameraAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->focal_length, v, it);
}
msAPI void msCameraAFillFocusDistance(ms::CameraAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->focus_distance, v, it);
}

msAPI void msLightAFillColor(ms::LightAnimation *self, Keyframe *x, Keyframe *y, Keyframe *z, Keyframe *w, InterpolationMode it)
{
    FillCurves(self->color, x, y, z, w, it);
}
msAPI void msLightAFillIntensity(ms::LightAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->intensity, v, it);
}
msAPI void msLightAFillRange(ms::LightAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->range, v, it);
}
msAPI void msLightAFillSpotAngle(ms::LightAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->spot_angle, v, it);
}

msAPI void msMeshAFillBlendshapeWeight(ms::MeshAnimation *self, int bi, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->blendshapes[bi]->weight, v, it);
}

msAPI void msPointsAFillTime(ms::PointsAnimation *self, Keyframe *v, InterpolationMode it)
{
    FillCurve(self->time, v, it);
}
