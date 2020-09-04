#include "pch.h"
#include "MeshSync/msMisc.h" 

namespace ms {

bool StartsWith(const std::string& a, const char *b)
{
    if (!b)
        return false;
    size_t n = std::strlen(b);
    return std::strncmp(a.c_str(), b, n) == 0;
}

bool StartsWith(const std::string& a, const std::string& b) {
    return (a.rfind(b, 0) == 0);
}

//----------------------------------------------------------------------------------------------------------------------

#ifndef msRuntime
bool FileToByteArray(const char *path, RawVector<char> &dst)
{
    if (!path)
        return false;

    const Poco::File file = Poco::File(path);
    if (!file.exists())
        return false;

    // note: FILE or std::fstream may fail to open files if path contains multi-byte characters
    Poco::FileStream f(path, std::ios::in);
    if (!f)
        return false;

    const Poco::File::FileSize size = file.getSize();
    dst.resize_discard(static_cast<size_t>(size));
    f.read(dst.data(), static_cast<size_t>(size));
    return true;
}

bool FileToByteArray(const char *path, SharedVector<char>& out)
{
    return FileToByteArray(path, out.as_raw());
}

bool ByteArrayToFile(const char *path, const RawVector<char> &data)
{
    return ByteArrayToFile(path, data.cdata(), data.size());
}
bool ByteArrayToFile(const char *path, const SharedVector<char>& data)
{
    return ByteArrayToFile(path, data.as_craw());
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

void FindFilesSortedByLastModified(const std::string& path, std::multimap<uint64_t, std::string>& ret) {

    using namespace std;
    Poco::DirectoryIterator dir_itr(path);
    Poco::DirectoryIterator	end;

    ret.clear();
    while(dir_itr!= end) {
        if (dir_itr->isDirectory()) {
            continue;
        }
        ret.insert(multimap<uint64_t, string>::value_type(dir_itr->getLastModified().raw(), dir_itr->path()));
        ++dir_itr;
    }
}
#endif // msRuntime

} // namespace ms
