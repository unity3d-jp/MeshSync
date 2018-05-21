#pragma once

namespace ms {

template<class T> struct has_serializer { static const bool result = false; };
#define HasSerializer(T) template<> struct has_serializer<T> { static const bool result = true; };

#define Size(V) ret += ssize(V);
#define Write(V) write(os, V);
#define Read(V) read(is, V);

} // namespace ms
