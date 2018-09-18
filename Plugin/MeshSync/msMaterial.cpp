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

Material::Material()
{
    flags.has_color = 1;
}
Material::~Material() {}

#define EachMember(F)\
    F(id) F(name) F(flags) F(color) F(emission) F(metalic) F(smoothness)\
    F(color_map) F(metallic_map) F(emission_map) F(normal_map)

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
    flags = { 0 };
    flags.has_color = 1;

    color = float4::one();
    emission = float4::zero();
    metalic = 0.0f;
    smoothness = 0.5f;

    color_map = 0;
    metallic_map = 0;
    emission_map = 0;
    normal_map = 0;
}

bool Material::operator==(const Material& v) const
{
    return
        name == v.name &&
        color == v.color &&
        emission == v.emission &&
        metalic == v.metalic &&
        smoothness == v.smoothness &&
        color_map == v.color_map &&
        emission_map == v.emission_map &&
        metallic_map == v.metallic_map &&
        normal_map == v.normal_map;
}
bool Material::operator!=(const Material& v) const
{
    return !(*this == v);
}

void    Material::setColor(float4 v)    { color = v; flags.has_color = 1; }
float4  Material::getColor() const      { return color; }
void    Material::setEmission(float4 v) { emission = v; flags.has_emission = 1; }
float4  Material::getEmission() const   { return emission; }
void    Material::setMetallic(float v)  { metalic = v; flags.has_metallic = 1; }
float   Material::getMetallic() const   { return metalic; }
void    Material::setSmoothness(float v){ smoothness = v; flags.has_smoothness = 1; }
float   Material::getSmoothness() const { return smoothness; }

void    Material::setColorMap(int v)    { color_map = v; flags.has_color_map = (v >= 0 ? 1 : 0); }
int     Material::getColorMap() const   { return color_map; }
void    Material::setEmissionMap(int v) { emission_map = v; flags.has_emission_map = (v >= 0 ? 1 : 0); }
int     Material::getEmissionMap() const{ return emission_map; }
void    Material::setMetallicMap(int v) { metallic_map = v; flags.has_metallic_map = (v >= 0 ? 1 : 0); }
int     Material::getMetallicMap() const{ return metallic_map; }
void    Material::setNormalMap(int v)   { normal_map = v; flags.has_normal_map = (v >= 0 ? 1 : 0); }
int     Material::getNormalMap() const  { return normal_map; }


#undef EachMember

} // namespace ms
