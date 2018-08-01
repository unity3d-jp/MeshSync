#include "pch.h"
#include "msMaterial.h"
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
    FILE *f = fopen(path, "wb");
    if (!f)
        return false;
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);
    return true;
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
    id = 0;
    name.clear();

    type = TextureType::Default;
    format = TextureFormat::Unknown;
    width = height = 0;
    data.clear();
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


std::shared_ptr<Material> Material::create(std::istream & is)
{
    auto ret = Pool<Material>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

Material::Material() {}
Material::~Material() {}

#define EachMember(F)\
    F(id) F(name) F(color) F(emission) F(metalic) F(smoothness)\
    F(color_tid) F(metallic_tid) F(emission_tid) F(normal_tid)

uint32_t Material::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}
void Material::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}
void Material::deserialize(std::istream& is)
{
    EachMember(msRead);
}

void Material::clear()
{
    id = 0;
    name.clear();

    color = float4::one();
    emission = { 0.0f, 0.0f, 0.0f, -1.0f };
    metalic = -1.0f;
    smoothness = -1.0f;

    color_tid = 0;
    metallic_tid = 0;
    emission_tid = 0;
    normal_tid = 0;
}

bool Material::operator==(const Material& v) const
{
    return
        name == v.name &&
        color == v.color &&
        emission == v.emission &&
        metalic == v.metalic &&
        smoothness == v.smoothness;
}
bool Material::operator!=(const Material& v) const
{
    return !(*this == v);
}

#undef EachMember

} // namespace ms
