#pragma once
#include "MeshUtils/MeshUtils.h"

namespace ms {

bool StartWith(const std::string& a, const char *b);
bool FileToByteArray(const char *path, RawVector<char> &out);
bool ByteArrayToFile(const char *path, const RawVector<char> &data);
bool ByteArrayToFile(const char *path, const char *data, size_t size);
bool FileExists(const char *path);
uint64_t FileMTime(const char *path);

} // namespace ms
