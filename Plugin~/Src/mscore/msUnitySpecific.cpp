#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "msCoreAPI.h"
#include "MeshSync/SceneGraph/msAnimation.h"

using namespace mu;

// on Unity, the structure of Keyframe is different on editor and runtime. also, some members are added on Unity 2018.1.
// we need to support all of Unity 2017.4 or later keyframe types.

// pre-Unity 2018.1 runtime Keyframe
struct Keyframe_R
{
    float time;
    float value;
    float in_tangent;
    float out_tangent;

    int   getTangentMode() const { return 0; }
    void  setTangentMode(int v) {}

    int   getWeightedMode() const { return 0; }
    void  setWeightedMode(int v) {}
    float getInWeight() const { return 0.0f; }
    void  setInWeight(float v) {}
    float getOutWeight() const { return 0.0f; }
    void  setOutWeight(float v) {}
};

// pre-Unity 2018.1 editor Keyframe
struct Keyframe_E
{
    float time;
    float value;
    float in_tangent;
    float out_tangent;

    int tangent_mode; // editor only

    int   getTangentMode() const { return tangent_mode; }
    void  setTangentMode(int v) { tangent_mode = v; }

    int   getWeightedMode() const { return 0; }
    void  setWeightedMode(int v) {}
    float getInWeight() const { return 0.0f; }
    void  setInWeight(float v) {}
    float getOutWeight() const { return 0.0f; }
    void  setOutWeight(float v) {}
};

// Unity 2018.1+ runtime Keyframe
struct Keyframe_RW
{
    float time;
    float value;
    float in_tangent;
    float out_tangent;

    int weighted_mode; // Unity 2018.1+ only
    float in_weight;   // 
    float out_weight;  // 

    int   getTangentMode() const { return 0; }
    void  setTangentMode(int v) {}

    int   getWeightedMode() const { return weighted_mode; }
    void  setWeightedMode(int v) { weighted_mode = v; }
    float getInWeight() const { return in_weight; }
    void  setInWeight(float v) { in_weight = v; }
    float getOutWeight() const { return out_weight; }
    void  setOutWeight(float v) { out_weight = v; }
};

// Unity 2018.1+ editor Keyframe
struct Keyframe_EW
{
    float time;
    float value;
    float in_tangent;
    float out_tangent;

    int tangent_mode; // editor only

    int weighted_mode; // Unity 2018.1+ only
    float in_weight;   // 
    float out_weight;  // 

    int   getTangentMode() const { return tangent_mode; }
    void  setTangentMode(int v) { tangent_mode = v; }

    int   getWeightedMode() const { return weighted_mode; }
    void  setWeightedMode(int v) { weighted_mode = v; }
    float getInWeight() const { return in_weight; }
    void  setInWeight(float v) { in_weight = v; }
    float getOutWeight() const { return out_weight; }
    void  setOutWeight(float v) { out_weight = v; }
};

// dummy for not yet supported Keyframe
struct UnknownKF {};


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
static const int kBrokenMask = 1 << 0;
static const int kLeftTangentMask = 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4;
static const int kRightTangentMask = 1 << 5 | 1 << 6 | 1 << 7 | 1 << 8;
static const float kTimeEpsilon = 0.00001f;
static const float kDefaultWeight = 1.0f / 3.0f;
static const float kCurveTimeEpsilon = 0.00001f;

template<class KF>
static inline float LinearTangent(IArray<KF>& curve, int i1, int i2)
{
    const auto& k1 = curve[i1];
    const auto& k2 = curve[i2];

    float dt = k2.time - k1.time;
    if (std::abs(dt) < kTimeEpsilon)
        return 0.0f;

    return (k2.value - k1.value) / dt;
}

template<class KF>
static inline TangentMode GetLeftTangentMode(const KF& key)
{
    return static_cast<TangentMode>((key.getTangentMode() & kLeftTangentMask) >> 1);
}

template<class KF>
static inline TangentMode GetRightTangentMode(const KF& key)
{
    return static_cast<TangentMode>((key.getTangentMode() & kRightTangentMask) >> 5);
}

static inline float SafeDiv(float y, float x)
{
    if (std::abs(x) > kCurveTimeEpsilon)
        return y / x;
    else
        return 0;
}

template<class KF>
static void RecalculateSplineSlopeT(IArray<KF> curve, int key, float b = 0.0f)
{
    if (curve.size() < 2)
        return;

    if (key == 0) {
        float dx = curve[1].time - curve[0].time;
        float dy = curve[1].value - curve[0].value;
        float m = dy / dx;
        curve[key].in_tangent = m;
        curve[key].out_tangent = m;
        curve[key].setOutWeight(kDefaultWeight);
    }
    else if (key == curve.size() - 1) {
        float dx = curve[key].time - curve[key - 1].time;
        float dy = curve[key].value - curve[key - 1].value;
        float m = dy / dx;
        curve[key].in_tangent = m;
        curve[key].out_tangent = m;
        curve[key].setInWeight(kDefaultWeight);
    }
    else {
        float dx1 = curve[key].time - curve[key - 1].time;
        float dy1 = curve[key].value - curve[key - 1].value;

        float dx2 = curve[key + 1].time - curve[key].time;
        float dy2 = curve[key + 1].value - curve[key].value;

        float m1 = SafeDiv(dy1, dx1);
        float m2 = SafeDiv(dy2, dx2);

        float m = (1.0f + b) * 0.5f * m1 + (1.0f - b) * 0.5f * m2;
        curve[key].in_tangent = m;
        curve[key].out_tangent = m;
        curve[key].setInWeight(kDefaultWeight);
        curve[key].setOutWeight(kDefaultWeight);
    }
}

template<class KF>
static void EnsureQuaternionContinuityAndRecalculateSlope(KF *x, KF *y, KF *z, KF *w, int key_count)
{
    auto get_value = [&](int i) -> quatf {
        return { x[i].value, y[i].value, z[i].value, w[i].value };
    };
    auto set_value = [&](int i, quatf v) {
        x[i].value = v.x;
        y[i].value = v.y;
        z[i].value = v.z;
        w[i].value = v.w;
    };

    auto last = get_value(key_count - 1);
    for (int i = 0; i < key_count; i++) {
        auto cur = get_value(i);
        if (dot(cur, last) < 0.0f)
            cur = { -cur.x, -cur.y, -cur.z, -cur.w };
        last = cur;
        set_value(i, cur);
    }

    for (int i = 0; i < key_count; i++) {
        size_t n = key_count;
        RecalculateSplineSlopeT<KF>({ x, n }, i);
        RecalculateSplineSlopeT<KF>({ y, n }, i);
        RecalculateSplineSlopeT<KF>({ z, n }, i);
        RecalculateSplineSlopeT<KF>({ w, n }, i);
    }
}

template<class KF>
static inline void SmoothTangents(IArray<KF>& curve, int index, float bias)
{
    if (curve.size() < 2)
        return;

    auto& key = curve[index];
    if (index == 0) {
        key.in_tangent = key.out_tangent = 0;

        if (curve.size() > 1) {
            key.setInWeight(kDefaultWeight);
            key.setOutWeight(kDefaultWeight);
        }
    }
    else if (index == curve.size() - 1) {
        key.in_tangent = key.out_tangent = 0;

        if (curve.size() > 1) {
            key.setInWeight(kDefaultWeight);
            key.setOutWeight(kDefaultWeight);
        }
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

        key.setInWeight(kDefaultWeight);
        key.setOutWeight(kDefaultWeight);
    }
}

template<class KF>
static inline void UpdateTangents(IArray<KF>& curve, int index)
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

template<class KF>
static void SetTangentMode(KF *key, int n, InterpolationMode im)
{
    TangentMode tangent_mode;
    switch (im) {
    case Linear: tangent_mode = kTangentModeLinear; break;
    case Constant: tangent_mode = kTangentModeConstant; break;
    default: tangent_mode = kTangentModeClampedAuto; break;
    }

    for (int i = 0; i < n; ++i) {
        auto& k = key[i];
        int tm = k.getTangentMode();
        tm |= kBrokenMask;
        tm &= ~kLeftTangentMask;
        tm |= (int)tangent_mode << 1;
        tm &= ~kRightTangentMask;
        tm |= (int)tangent_mode << 5;
        k.setTangentMode(tm);

        k.setInWeight(kDefaultWeight);
        k.setOutWeight(kDefaultWeight);
    }

    IArray<KF> curve{ key, (size_t)n };
    for (int i = 0; i < n; ++i)
        UpdateTangents(curve, i);
}

template<class KF>
static void FillCurveI(const ms::TAnimationCurve<int>& src, void *x_, InterpolationMode it)
{
    auto *x = (KF*)x_;
    int n = (int)src.size();
    for (int i = 0; i < n; ++i) {
        const auto v = src[i];
        x[i].time = v.time;
        x[i].value = (float)v.value;
    }
    SetTangentMode(x, n, it);
}
template<> void FillCurveI<UnknownKF>(const ms::TAnimationCurve<int>& src, void *x_, InterpolationMode it) {}

template<class KF>
static void FillCurveF(const ms::TAnimationCurve<float>& src, void *x_, InterpolationMode it)
{
    auto *x = (KF*)x_;
    int n = (int)src.size();
    for (int i = 0; i < n; ++i) {
        const auto v = src[i];
        x[i].time = v.time;
        x[i].value = v.value;
    }
    SetTangentMode(x, n, it);
}
template<> void FillCurveF<UnknownKF>(const ms::TAnimationCurve<float>& src, void *x_, InterpolationMode it) {}

template<class KF>
static void FillCurvesF2(const ms::TAnimationCurve<float2>& src, void *x_, void *y_, InterpolationMode it)
{
    auto *x = (KF*)x_;
    auto *y = (KF*)y_;
    int n = (int)src.size();
    for (int i = 0; i < n; ++i) {
        const auto v = src[i];
        x[i].time = v.time;
        x[i].value = v.value.x;
        y[i].time = v.time;
        y[i].value = v.value.y;
    }
    SetTangentMode(x, n, it);
    SetTangentMode(y, n, it);
}
template<> void FillCurvesF2<UnknownKF>(const ms::TAnimationCurve<float2>& src, void *x_, void *y_, InterpolationMode it) {}

template<class KF>
static void FillCurvesF3(const ms::TAnimationCurve<float3>& src, void *x_, void *y_, void *z_, InterpolationMode it)
{
    auto *x = (KF*)x_;
    auto *y = (KF*)y_;
    auto *z = (KF*)z_;

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
template<> void FillCurvesF3<UnknownKF>(const ms::TAnimationCurve<float3>& src, void *x_, void *y_, void *z_, InterpolationMode it) {}

template<class KF>
static void FillCurvesF4(const ms::TAnimationCurve<float4>& src, void *x_, void *y_, void *z_, void *w_, InterpolationMode it)
{
    auto *x = (KF*)x_;
    auto *y = (KF*)y_;
    auto *z = (KF*)z_;
    auto *w = (KF*)w_;

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
template<> void FillCurvesF4<UnknownKF>(const ms::TAnimationCurve<float4>& src, void *x_, void *y_, void *z_, void *w_, InterpolationMode it) {}

template<class KF>
static void FillCurvesQuat(const ms::TAnimationCurve<quatf>& src, void *x_, void *y_, void *z_, void *w_, InterpolationMode it)
{
    auto *x = (KF*)x_;
    auto *y = (KF*)y_;
    auto *z = (KF*)z_;
    auto *w = (KF*)w_;

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

    if (it == InterpolationMode::Smooth)
        EnsureQuaternionContinuityAndRecalculateSlope(x, y, z, w, n);
}
template<> void FillCurvesQuat<UnknownKF>(const ms::TAnimationCurve<quatf>& src, void *x_, void *y_, void *z_, void *w_, InterpolationMode it) {}

template<class KF>
static void FillCurvesEuler(const ms::TAnimationCurve<quatf>& src, void *x_, void *y_, void *z_, InterpolationMode it)
{
    KF *x = (KF*)x_;
    KF *y = (KF*)y_;
    KF *z = (KF*)z_;

    int n = (int)src.size();
    float3 prev;
    for (int i = 0; i < n; ++i) {
        const auto v = src[i];
        auto r = mu::to_euler_zxy(v.value) * mu::RadToDeg;
        if (i > 0) {
            // make continuous
            auto d = r - mu::mod(prev, 360.0f);
            auto x0 = mu::abs(d);
            auto x1 = mu::abs(d - 360.0f);
            auto x2 = mu::abs(d + 360.0f);

            if (x1.x < x0.x)
                r.x -= 360.0f;
            else if (x2.x < x0.x)
                r.x += 360.0f;

            if (x1.y < x0.y)
                r.y -= 360.0f;
            else if (x2.y < x0.y)
                r.y += 360.0f;

            if (x1.z < x0.z)
                r.z -= 360.0f;
            else if (x2.z < x0.z)
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
template<> void FillCurvesEuler<UnknownKF>(const ms::TAnimationCurve<quatf>& src, void *x_, void *y_, void *z_, InterpolationMode it) {}


// thanks: http://techblog.sega.jp/entry/2016/11/28/100000
template<class KF>
class AnimationCurveKeyReducer
{
public:
    static RawVector<KF> apply(const IArray<KF>& keys, float eps, bool erase_flat_curves)
    {
        RawVector<KF> ret;
        ret.reserve(keys.size());
        if (keys.size() <= 2)
            ret.assign(keys.begin(), keys.end());
        else
            genNewKeys(keys, eps, ret);

        if (erase_flat_curves && isFlat(ret))
            ret.clear();
        return ret;
    }

private:
    static bool isFlat(RawVector<KF>& keys)
    {
        if (keys.size() <= 1)
            return true;

        auto n = (int)keys.size();
        auto base = keys.front().value;
        for (int i = 1; i < n; ++i)
            if (keys[i].value != base)
                return false;
        return true;
    }

    static void genNewKeys(const IArray<KF>& keys, float eps, RawVector<KF>& ret)
    {
        ret.push_back(keys.front());

        int end = (int)keys.size() - 1;
        for (int k = 0, i = 1; i < end; i++) {
            if (!nearEqualToEvaluated(keys[k], keys[i + 1], keys[i], eps)) {
                k = i;
                ret.push_back(keys[k]);
            }
        }
        ret.push_back(keys.back());
    }

    static bool nearEqualToEvaluated(const KF& key1, const KF& key2, const KF& comp, float eps)
    {
        float val1 = evaluate(key1, key2, comp.time);
        if (!near_equal(val1, comp.value, eps))
            return false;

        float time = key1.time + (comp.time - key1.time) * 0.5f;
        val1 = evaluate(key1, comp, time);
        float val2 = evaluate(key1, key2, time);
        return near_equal(val1, val2, eps);
    }

    static float evaluate(const KF& key1, const KF& key2, float time)
    {
        if (std::isinf(key1.out_tangent))
            return key1.value;

        float kd = key2.time - key1.time;
        float vd = key2.value - key1.value;
        float t = (time - key1.time) / kd;

        float a = -2 * vd + kd * (key1.out_tangent + key2.in_tangent);
        float b = 3 * vd - kd * (2 * key1.out_tangent + key2.in_tangent);
        float c = kd * key1.out_tangent;

        return key1.value + t * (t * (a * t + b) + c);
    }
};

template<class KF>
static int KeyframeReductionA(void *keys_, int key_count, float threshold, bool erase_flat_curves)
{
    if (!keys_ || key_count == 0)
        return 0;

    auto *keys = (KF*)keys_;
    auto new_keys = AnimationCurveKeyReducer<KF>().apply({ keys, (size_t)key_count }, threshold, erase_flat_curves);
    new_keys.copy_to(keys);
    return (int)new_keys.size();
}
template<> int KeyframeReductionA<UnknownKF>(void *keys_, int key_count, float threshold, bool erase_flat_curves) { return 0; }

template<class KF>
static void KeyframeReductionV(RawVector<char>& idata, float threshold, bool erase_flat_curves)
{
    if (idata.empty())
        return;

    auto *keys = (const KF*)idata.cdata();
    size_t key_count = idata.size() / sizeof(KF);

    auto new_keys = AnimationCurveKeyReducer<KF>().apply({ keys, key_count }, threshold, erase_flat_curves);
    idata.assign((const char*)new_keys.cdata(), new_keys.size_in_byte());
}
template<> void KeyframeReductionV<UnknownKF>(RawVector<char>& idata, float threshold, bool erase_flat_curves) {}


struct FunctionSet
{
    void(*FillCurveI)(const ms::TAnimationCurve<int>& src, void *x_, InterpolationMode it);
    void(*FillCurveF)(const ms::TAnimationCurve<float>& src, void *x_, InterpolationMode it);
    void(*FillCurvesF2)(const ms::TAnimationCurve<float2>& src, void *x_, void *y_, InterpolationMode it);
    void(*FillCurvesF3)(const ms::TAnimationCurve<float3>& src, void *x_, void *y_, void *z_, InterpolationMode it);
    void(*FillCurvesF4)(const ms::TAnimationCurve<float4>& src, void *x_, void *y_, void *z_, void *w_, InterpolationMode it);
    void(*FillCurvesQuat)(const ms::TAnimationCurve<quatf>& src, void *x_, void *y_, void *z_, void *w_, InterpolationMode it);
    void(*FillCurvesEuler)(const ms::TAnimationCurve<quatf>& src, void *x_, void *y_, void *z_, InterpolationMode it);

    int(*KeyframeReductionA)(void *keys_, int key_count, float threshold, bool erase_flat_curves);
    void(*KeyframeReductionV)(RawVector<char>& idata, float threshold, bool erase_flat_curves);
};
static FunctionSet g_fs;

template<class KF>
static void SetupFunctionSet()
{
    g_fs = {
        &FillCurveI<KF>,
        &FillCurveF<KF>,
        &FillCurvesF2<KF>,
        &FillCurvesF3<KF>,
        &FillCurvesF4<KF>,
        &FillCurvesQuat<KF>,
        &FillCurvesEuler<KF>,
        &KeyframeReductionA<KF>,
        &KeyframeReductionV<KF>,
    };
}

static int g_sizeof_keyframe = 0;

msAPI void msSetSizeOfKeyframe(int v)
{
    g_sizeof_keyframe = v;

    switch (g_sizeof_keyframe) {
        case sizeof(Keyframe_EW): SetupFunctionSet<Keyframe_EW>(); break;
        case sizeof(Keyframe_RW): SetupFunctionSet<Keyframe_EW>(); break;
        case sizeof(Keyframe_E): SetupFunctionSet<Keyframe_E>(); break;
        case sizeof(Keyframe_R): SetupFunctionSet<Keyframe_R>(); break;
        default: SetupFunctionSet<UnknownKF>(); break;
    }
}

static void ConvertCurve(ms::AnimationCurve& curve, InterpolationMode it)
{
    size_t data_size = g_sizeof_keyframe * curve.size();

    if (curve.data_flags.force_constant)
        it = InterpolationMode::Constant;

    char *dst[4]{};
    auto alloc = [&](int n) {
        curve.idata.resize(n);
        for (int i = 0; i < n; ++i) {
            curve.idata[i].resize_zeroclear(data_size);
            dst[i] = curve.idata[i].data();
        }
    };

    switch (curve.data_type) {
    case ms::Animation::DataType::Int:
        alloc(1);
        g_fs.FillCurveI(ms::TAnimationCurve<int>(curve), dst[0], it);
        break;
    case ms::Animation::DataType::Float:
        alloc(1);
        g_fs.FillCurveF(ms::TAnimationCurve<float>(curve), dst[0], it);
        break;
    case ms::Animation::DataType::Float2:
        alloc(2);
        g_fs.FillCurvesF2( ms::TAnimationCurve<float2>(curve), dst[0], dst[1], it);
        break;
    case ms::Animation::DataType::Float3:
        alloc(3);
        g_fs.FillCurvesF3(ms::TAnimationCurve<float3>(curve), dst[0], dst[1], dst[2], it);
        break;
    case ms::Animation::DataType::Float4:
        alloc(4);
        g_fs.FillCurvesF4(ms::TAnimationCurve<float4>(curve), dst[0], dst[1], dst[2], dst[3], it);
        break;
    case ms::Animation::DataType::Quaternion:
        if (it == InterpolationMode::Linear || it == InterpolationMode::Constant) {
            alloc(3);
            g_fs.FillCurvesEuler(ms::TAnimationCurve<quatf>(curve), dst[0], dst[1], dst[2], it);
        }
        else {
            alloc(4);
            g_fs.FillCurvesQuat(ms::TAnimationCurve<quatf>(curve), dst[0], dst[1], dst[2], dst[3], it);
        }
        break;
    default:
        break;
    }
}
static void ConvertCurves(ms::Animation& anim, InterpolationMode it)
{
    int n = (int)anim.curves.size();
    for (int i = 0; i != n; ++i)
        ConvertCurve(*anim.curves[i], it);
}

static void KeyframeReduction(ms::AnimationCurve& curve, float threshold, bool erase_flat_curves)
{
    for (auto& idata : curve.idata) {
        g_fs.KeyframeReductionV(idata, threshold, erase_flat_curves);
    }
}

static void KeyframeReduction(ms::Animation& anim, float threshold, bool erase_flat_curves)
{
    auto idata_empty = [](auto& curve) {
        for (auto& id : curve.idata)
            if (!id.empty())
                return false;
        return true;
    };

    for (auto& curve : anim.curves) {
        KeyframeReduction(*curve, threshold, erase_flat_curves);
        if (idata_empty(*curve)) {
            curve->idata.clear();
            curve->data.clear();
        }
    }
    anim.clearEmptyCurves();
}

msAPI int msKeyframeReduction(void *kfs, int kf_count, float threshold, bool erase_flat_curves)
{
    return g_fs.KeyframeReductionA(kfs, kf_count, threshold, erase_flat_curves);
}

msAPI int msCurveGetNumElements(ms::AnimationCurve *self)
{
    return (int)self->idata.size();
}
msAPI int msCurveGetNumKeys(ms::AnimationCurve *self, int i)
{
    return (int)self->idata[i].size() / g_sizeof_keyframe;
}
msAPI void msCurveCopy(ms::AnimationCurve *self, int i, void *dst)
{
    memcpy(dst, self->idata[i].cdata(), self->idata[i].size());
}
msAPI void msCurveConvert(ms::AnimationCurve *self, InterpolationMode it)
{
    ConvertCurve(*self, it);
}
msAPI void msCurveKeyframeReduction(ms::AnimationCurve *self, float threshold, bool erase_flat_curves)
{
    KeyframeReduction(*self, threshold, erase_flat_curves);
}

msAPI void msAnimationConvert(ms::Animation *self, InterpolationMode it)
{
    ConvertCurves(*self, it);
}
msAPI void msAnimationKeyframeReduction(ms::Animation *self, float threshold, bool erase_flat_curves)
{
    KeyframeReduction(*self, threshold, erase_flat_curves);
}

msAPI void msAnimationClipConvert(ms::AnimationClip *self, InterpolationMode it)
{
    int n = (int)self->animations.size();
    mu::parallel_for(0, n, 16,
        [&](int i) {
            ConvertCurves(*self->animations[i], it);
        });
}
msAPI void msAnimationClipKeyframeReduction(ms::AnimationClip *self, float threshold, bool erase_flat_curves)
{
    int n = (int)self->animations.size();
    mu::parallel_for(0, n, 16,
        [&](int i) {
            KeyframeReduction(*self->animations[i], threshold, erase_flat_curves);
        });
    self->clearEmptyAnimations();
}
#undef Switch
