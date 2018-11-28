#include "pch.h"
#include "msTexture.h"
#include "msMisc.h"

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



#define EachMember(F)  F(type) F(format) F(width) F(height) F(data)

std::shared_ptr<Texture> Texture::create(std::istream& is)
{
    return std::static_pointer_cast<Texture>(Asset::create(is));
}

Texture::Texture() {}
Texture::~Texture() {}

AssetType Texture::getAssetType() const
{
    return AssetType::Texture;
}

void Texture::serialize(std::ostream& os) const
{
    super::serialize(os);
    EachMember(msWrite);
}

void Texture::deserialize(std::istream& is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void Texture::clear()
{
    super::clear();
    type = TextureType::Default;
    format = TextureFormat::Unknown;
    width = height = 0;
    vclear(data);
}

uint64_t Texture::hash() const
{
    uint64_t ret = super::hash();
    ret += vhash(data);
    return ret;
}

uint64_t Texture::checksum() const
{
    uint64_t ret = super::checksum();
    ret += csum((int)type);
    ret += csum((int)format);
    ret += csum(width);
    ret += csum(height);
    ret += csum(data);
    return ret;
}

void Texture::setData(const void *src)
{
    size_t data_size = width * height * GetPixelSize(format);
    data.assign((const char*)src, (const char*)src + data_size);
}

void Texture::getData(void *dst) const
{
    if (!dst)
        return;
    data.copy_to((char*)dst);
}

bool Texture::readFromFile(const char *path)
{
    if (!path)
        return false;
    if (FileToByteArray(path, data)) {
        name = GetFilename(path);
        format = TextureFormat::RawFile;
        return true;
    }
    return false;
}

bool Texture::writeToFile(const char *path) const
{
    if (!path)
        return false;
    return ByteArrayToFile(path, data);
}

#undef EachMember

} // namespace ms
