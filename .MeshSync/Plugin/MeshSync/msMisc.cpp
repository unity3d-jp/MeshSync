#include "pch.h"
#include "msMisc.h"

namespace ms {

bool StartWith(const std::string& a, const char *b)
{
    if (!b)
        return false;
    size_t n = std::strlen(b);
    return std::strncmp(a.c_str(), b, n) == 0;
}

bool FileToByteArray(const char *path, RawVector<char> &dst)
{
    if (!path)
        return false;

    // note: FILE or std::fstream may fail to open files if path contains multi-byte characters
    Poco::FileStream f(path, std::ios::in);
    if (!f)
        return false;
    auto size = Poco::File(path).getSize();
    dst.resize_discard((size_t)size);
    f.read(dst.data(), (size_t)size);
    return true;
}

bool ByteArrayToFile(const char *path, const RawVector<char> &data)
{
    return ByteArrayToFile(path, data.data(), data.size());
}

bool ByteArrayToFile(const char *path, const char *data, size_t size)
{
    if (!path)
        return false;

    Poco::FileStream f(path, std::ios::out);
    if (!f)
        return false;
    f.write(data, size);
    return true;
}

bool FileExists(const char *path)
{
    if (!path || *path == '\0')
        return false;

    try {
        // this is fater than using fopen()
        return Poco::File(path).exists();
    }
    catch (...) {
        return false;
    }
}

uint64_t FileMTime(const char *path)
{
    if (!FileExists(path))
        return 0;

    try {
        Poco::File f(path);
        return f.getLastModified().raw();
    }
    catch (...) {
        return 0;
    }
}

} // namespace ms
