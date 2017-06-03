#pragma once

namespace mu {

using nanosec = uint64_t;
nanosec Now();

void Print(const char *fmt, ...);
void Print(const wchar_t *fmt, ...);

std::string ToUTF8(const char *src);
std::string ToUTF8(const std::string& src);
std::string ToANSI(const char *src);
std::string ToANSI(const std::string& src);

} // namespace mu
