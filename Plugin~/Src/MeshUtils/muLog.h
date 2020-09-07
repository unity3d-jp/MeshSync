#pragma once

namespace mu {

#define muLogInfo(...)    ::mu::Print("MeshSync info: " __VA_ARGS__)
#define muLogWarning(...) ::mu::Print("MeshSync warning: " __VA_ARGS__)
#define muLogError(...)   ::mu::Print("MeshSync error: " __VA_ARGS__)

void Print(const char *fmt, ...);
void Print(const wchar_t *fmt, ...);

} // namespace mu
