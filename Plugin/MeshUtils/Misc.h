#pragma once

namespace mu {

template < typename T, size_t N >
size_t countof(T(&arr)[N]) { return std::extent< T[N] >::value; }

using nanosec = uint64_t;
nanosec Now();

void Print(const char *fmt, ...);
void Print(const wchar_t *fmt, ...);

std::string ToUTF8(const char *src);
std::string ToUTF8(const std::string& src);
std::string ToANSI(const char *src);
std::string ToANSI(const std::string& src);

} // namespace mu
