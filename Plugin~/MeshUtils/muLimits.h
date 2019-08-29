#pragma once
#include "muMath.h"

namespace mu {

template<class T> struct inf_impl;

// note: value() is static functions because making it constants cause link error on gcc.
template<> struct inf_impl<float>
{
    static constexpr float value() { return std::numeric_limits<float>::infinity(); }
};

template<> struct inf_impl<double>
{
    static constexpr double value() { return std::numeric_limits<double>::infinity(); }
};

template<class T> struct inf_impl<tvec2<T>>
{
    static constexpr tvec2<T> value() { return {inf_impl<T>::value(), inf_impl<T>::value()}; };
};

template<class T> struct inf_impl<tvec3<T>>
{
    static constexpr tvec3<T> value() { return { inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value() }; }
};

template<class T> struct inf_impl<tvec4<T>>
{
    static constexpr tvec4<T> value() { return { inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value() }; };
};

template<class T> struct inf_impl<tquat<T>>
{
    static constexpr tquat<T> value() { return { inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value() }; };
};

template<class T> struct inf_impl<tmat2x2<T>>
{
    static constexpr tmat2x2<T> value()
    {
        return { {
            { inf_impl<T>::value(), inf_impl<T>::value() },
            { inf_impl<T>::value(), inf_impl<T>::value() },
        } };
    }
};

template<class T> struct inf_impl<tmat3x3<T>>
{
    static constexpr tmat3x3<T> value()
    {
        return { {
            { inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value() },
            { inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value() },
            { inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value() },
        } };
    }
};

template<class T> struct inf_impl<tmat4x4<T>>
{
    static constexpr tmat4x4<T> value()
    {
        return { {
            { inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value() },
            { inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value() },
            { inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value() },
            { inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value(), inf_impl<T>::value() },
        } };
    }
};

template<class T> inline T inf() { return inf_impl<T>::value(); }
template<class T> inline bool is_inf(const T& v) { return v == inf_impl<T>::value(); }

} // namespace mu
