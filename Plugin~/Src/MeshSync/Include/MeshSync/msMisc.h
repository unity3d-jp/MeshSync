#pragma once
#include "MeshUtils/MeshUtils.h"

namespace ms {

bool StartsWith(const std::string& a, const char *b);
bool StartsWith(const std::string& a, const std::string& b);
#ifndef msRuntime
bool FileToByteArray(const char *path, RawVector<char> &out);
bool FileToByteArray(const char *path, SharedVector<char> &out);
bool ByteArrayToFile(const char *path, const RawVector<char> &data);
bool ByteArrayToFile(const char *path, const SharedVector<char> &data);
bool ByteArrayToFile(const char *path, const char *data, size_t size);
bool FileExists(const char *path);
uint64_t FileMTime(const char *path);
void FindFilesSortedByLastModified(const std::string& path, std::multimap<uint64_t, std::string>& ret);
#endif // msRuntime

} // namespace ms
