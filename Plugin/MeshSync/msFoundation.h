#pragma once

namespace ms {

template<class T> struct has_serializer { static const bool result = false; };
#define HasSerializer(T) template<> struct has_serializer<T> { static const bool result = true; };

} // namespace ms
