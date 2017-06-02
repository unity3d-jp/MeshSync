#pragma once

namespace amath {}

#ifdef _MSC_VER
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <amp.h>
#include <amp_graphics.h>
#include <amp_math.h>

namespace amath
{
    using namespace concurrency;
    using namespace concurrency::graphics;
    using namespace concurrency::fast_math;


#define Def1A(A,F)                                                                         \
    template<typename T, std::enable_if_t<short_vector_traits<T>::size == 2>* = nullptr>\
    inline T A(T v) restrict(cpu, amp) {                                                \
        return { F(v.x), F(v.y) };                                                      \
    }                                                                                   \
    template<typename T, std::enable_if_t<short_vector_traits<T>::size == 3>* = nullptr>\
    inline T A(T l, T r) restrict(cpu, amp) {                                           \
        return { F(v.x), F(v.y), F(v.z) };                                              \
    }                                                                                   \
    template<typename T, std::enable_if_t<short_vector_traits<T>::size == 4>* = nullptr>\
    inline T A(T l, T r) restrict(cpu, amp) {                                           \
        return { F(v.x), F(v.y), F(v.z), F(v.w) };                                      \
    }

#define Def2A(A,F)                                                                         \
    template<typename T, std::enable_if_t<short_vector_traits<T>::size == 2>* = nullptr>\
    inline T A(T a, T b) restrict(cpu, amp) {                                           \
        return { F(a.x, b.x), F(a.y, b.y )};                                            \
    }                                                                                   \
    template<typename T, std::enable_if_t<short_vector_traits<T>::size == 3>* = nullptr>\
    inline T A(T a, T b) restrict(cpu, amp) {                                           \
        return { F(a.x, b.x), F(a.y, b.y), F(a.z, b.z) };                               \
    }                                                                                   \
    template<typename T, std::enable_if_t<short_vector_traits<T>::size == 4>* = nullptr>\
    inline T A(T a, T b) restrict(cpu, amp) {                                           \
        return { F(a.x, b.x), F(a.y, b.y), F(a.z, b.z), F(a.w, b.w) };                  \
    }

#define Def1(F) Def1A(F,F)
#define Def2(F) Def2A(F,F)

Def1A(abs, fabs)
Def1(round)
Def1(floor)
Def1(ceil)
Def2(min)
Def2(max)
Def1(rcp)
Def1(sqrt)
Def1(rsqrt)
Def1(sin)
Def1(cos)
Def1(tan)
Def1(asin)
Def1(acos)
Def1(atan)
Def2(atan2)
Def1(exp)
Def1(log)
Def2(pow)
Def2(mod)
Def1(frac)

#undef Def2
#undef Def1

    inline float abs(float v) restrict(cpu, amp) { return fabs(v); }

    template<typename T, std::enable_if_t<short_vector_traits<T>::size == 2>* = nullptr>
    inline typename short_vector_traits<T>::value_type dot(T l, T r) restrict(cpu, amp) {
        return l.x * r.x + l.y * r.y;
    }
    template<typename T, std::enable_if_t<short_vector_traits<T>::size == 3>* = nullptr>
    inline typename short_vector_traits<T>::value_type dot(T l, T r) restrict(cpu, amp) {
        return l.x * r.x + l.y * r.y + l.z * r.z;
    }
    template<typename T, std::enable_if_t<short_vector_traits<T>::size == 4>* = nullptr>
    inline typename short_vector_traits<T>::value_type dot(T l, T r) restrict(cpu, amp) {
        return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
    }

    template<typename T, std::enable_if_t<short_vector_traits<T>::size == 3>* = nullptr>
    inline T cross(T l, T r) restrict(cpu, amp) {
        return {
            l.y * r.z - l.z * r.y,
            l.z * r.x - l.x * r.z,
            l.x * r.y - l.y * r.x };
    }

    template<typename T>
    inline typename short_vector_traits<T>::value_type length_sq(T v) restrict(cpu, amp) {
        return dot(v, v);
    }

    template<typename T>
    inline typename short_vector_traits<T>::value_type length(T v) restrict(cpu, amp) {
        return sqrt(dot(v, v));
    }

    template<typename T>
    inline T normalize(T v) restrict(cpu, amp) {
        return v * rsqrt(dot(v, v));
    }


    #define Epsilon 1e-6

    inline bool ray_triangle_intersection(float_3 pos, float_3 dir, float_3 p1, float_3 p2, float_3 p3, float& distance) restrict(cpu, amp)
    {
        float_3 e1 = p2 - p1;
        float_3 e2 = p3 - p1;
        float_3 p = cross(dir, e2);
        float det = dot(e1, p);
        if (abs(det) < Epsilon) return false;
        float inv_det = 1.0f / det;
        float_3 t = pos - p1;
        float u = dot(t, p) * inv_det;
        if (u < 0 || u  > 1) return false;
        float_3 q = cross(t, e1);
        float v = dot(dir, q) * inv_det;
        if (v < 0 || u + v > 1) return false;

        distance = dot(e2, q) * inv_det;
        return true;
    }

}

#endif // _MSC_VER
