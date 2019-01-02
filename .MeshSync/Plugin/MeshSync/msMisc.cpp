#include "pch.h"
#include "msMisc.h"

namespace ms {

bool FileToByteArray(const char *path, RawVector<char> &dst)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return false;

    fseek(f, 0, SEEK_END);
    dst.resize_discard((size_t)ftell(f));
    fseek(f, 0, SEEK_SET);
    fread(dst.data(), 1, dst.size(), f);
    fclose(f);
    return true;
}

bool ByteArrayToFile(const char *path, const RawVector<char> &data)
{
    if (!path)
        return false;
    FILE *f = fopen(path, "wb");
    if (!f)
        return false;
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);
    return true;
}

bool ByteArrayToFile(const char * path, const char *data, size_t size)
{
    if (!path)
        return false;
    FILE *f = fopen(path, "wb");
    if (!f)
        return false;
    fwrite(data, 1, size, f);
    fclose(f);
    return true;
}

bool FileExists(const char *path)
{
    if (!path || *path == '\0')
        return false;

    try {
        // this is fater than using fopen()
        Poco::File f(path);
        return f.exists();
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
