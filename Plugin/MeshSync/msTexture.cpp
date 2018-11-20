#include "pch.h"
#include "msTexture.h"
#include "msSceneGraphImpl.h"

namespace ms {

int GetPixelSize(TextureFormat format)
{
    int t = (int)format & (int)TextureFormat::TypeMask;
    int c = (int)format & (int)TextureFormat::ChannelMask;
    switch (t) {
    case (int)TextureFormat::Type_u8:
        return c * 1;
        break;

    case (int)TextureFormat::Type_i16:
    case (int)TextureFormat::Type_f16:
        return c * 2;
        break;

    case (int)TextureFormat::Type_i32:
    case (int)TextureFormat::Type_f32:
        return c * 4;
        break;
    }
    return 0;
}

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


std::shared_ptr<Texture> Texture::create(std::istream & is)
{
    auto ret = Pool<Texture>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

Texture::Texture() {}
Texture::~Texture() {}

#define EachMember(F)\
    F(id) F(name) F(type) F(format) F(width) F(height) F(data)

uint32_t Texture::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}

void Texture::serialize(std::ostream & os) const
{
    EachMember(msWrite);
}

void Texture::deserialize(std::istream & is)
{
    EachMember(msRead);
}

void Texture::clear()
{
    id = InvalidID;
    name.clear();

    type = TextureType::Default;
    format = TextureFormat::Unknown;
    width = height = 0;
    data.clear();
}

uint64_t Texture::hash() const
{
    return vhash(data);
}

uint64_t Texture::checksum() const
{
    uint64_t ret = 0;
    ret += csum((int)type);
    ret += csum((int)format);
    ret += csum(width);
    ret += csum(height);
    ret += csum(data);
    return ret;
}

void Texture::setData(const void * src)
{
    size_t data_size = width * height * GetPixelSize(format);
    data.assign((const char*)src, (const char*)src + data_size);
}

void Texture::getData(void * dst) const
{
    if (!dst)
        return;
    data.copy_to((char*)dst);
}

bool Texture::writeToFile(const char * path)
{
    if (data.empty())
        return false;
    return ByteArrayToFile(path, data);
}

#undef EachMember

} // namespace ms
