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

bool ByteArrayToFile(const char * path, const char *data, size_t size)
{
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


std::shared_ptr<Material> Material::create(std::istream & is)
{
    auto ret = Pool<Material>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

Material::Material() {}
Material::~Material() {}

#define EachMember(F) F(id) F(name) F(shader) F(params)

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
#undef EachMember

void Material::clear()
{
    id = InvalidID;
    name.clear();
    shader = "Standard";
    params.clear();
}

uint64_t Material::checksum() const
{
    uint64_t ret = 0;
    ret += csum(shader);
    ret += csum(params);
    return ret;
}

bool Material::operator==(const Material& v) const
{
    return
        name == v.name &&
        shader == v.shader &&
        params == v.params;
}
bool Material::operator!=(const Material& v) const
{
    return !(*this == v);
}

int Material::getParamCount() const
{
    return (int)params.size();
}
MaterialParam* Material::getParam(int i)
{
    return &params[i];
}
MaterialParam* Material::findParam(const char *n)
{
    auto it = std::find_if(params.begin(), params.end(), [n](const MaterialParam& v) { return v.name == n; });
    return it != params.end() ? &(*it) : nullptr;
}
const MaterialParam* Material::getParam(int i) const
{
    return const_cast<Material*>(this)->getParam(i);
}
const MaterialParam* Material::findParam(const char *n) const
{
    return const_cast<Material*>(this)->findParam(n);
}

void Material::addParam(MaterialParam v)
{
    if (auto *p = findParam(v.name.c_str()))
        *p = v;
    else
        params.push_back(v);
}

void Material::eraseParam(const char *n)
{
    auto it = std::find_if(params.begin(), params.end(), [n](const MaterialParam& v) { return v.name == n; });
    if (it != params.end())
        params.erase(it);
}


#define EachMember(F) F(name) F(type) F(data)

uint32_t MaterialParam::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}

void MaterialParam::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}

void MaterialParam::deserialize(std::istream& is)
{
    EachMember(msRead);
}
uint64_t MaterialParam::checksum() const
{
    return mu::SumInt32(data.data(), data.size());
}
bool MaterialParam::operator==(const MaterialParam& v) const
{
    return
        name == v.name &&
        type == v.type &&
        data == v.data;
}
bool MaterialParam::operator!=(const MaterialParam& v) const
{
    return !(*this == v);
}
#undef EachMember


template<class T>
static inline void set_impl(RawVector<char>& dst, const T& v)
{
    dst.resize_discard(sizeof(T));
    (T&)dst[0] = v;
}
template<class T>
static inline void set_impl(RawVector<char>& dst, const T *v, int count)
{
    dst.resize_discard(sizeof(T) * count);
    memcpy(dst.data(), v, dst.size());
}

MaterialParam::MaterialParam() {}
MaterialParam::MaterialParam(const char *n, int v) : name(n), type(Type::Int) { set_impl(data, v); }
MaterialParam::MaterialParam(const char *n, float v) : name(n), type(Type::Float) { set_impl(data, v); }
MaterialParam::MaterialParam(const char *n, const float4& v) : name(n), type(Type::Vector) { set_impl(data, v); }
MaterialParam::MaterialParam(const char *n, const float4x4& v) : name(n), type(Type::Matrix) { set_impl(data, v); }
MaterialParam::MaterialParam(const char *n, const float *v, int c) : name(n), type(Type::FloatArray) { set_impl(data, v, c); }
MaterialParam::MaterialParam(const char *n, const float4 *v, int c) : name(n), type(Type::VectorArray) { set_impl(data, v, c); }
MaterialParam::MaterialParam(const char *n, const float4x4 *v, int c) : name(n), type(Type::MatrixArray) { set_impl(data, v, c); }
MaterialParam::MaterialParam(const char *n, TexturePtr v) : name(n), type(Type::Texture) { set_impl(data, v ? v->id : InvalidID); }
MaterialParam::MaterialParam(const char *n, Texture *v) : name(n), type(Type::Texture) { set_impl(data, v ? v->id : InvalidID); }

int MaterialParam::getArraySize() const
{
    switch (type) {
    case Type::Unknown: return 0;
    case Type::FloatArray: return (int)data.size() / sizeof(float);
    case Type::VectorArray: return (int)data.size() / sizeof(float4);
    case Type::MatrixArray: return (int)data.size() / sizeof(float4x4);
    default: return 1;
    }
}

int MaterialParam::getInt() const { return (int&)data[0]; }
float MaterialParam::getFloat() const { return (float&)data[0]; }
float4 MaterialParam::getVector() const { return (float4&)data[0]; }
float4x4 MaterialParam::getMatrix() const { return (float4x4&)data[0]; }
const float* MaterialParam::getFloatArray() const { return (float*)&data[0]; }
const float4* MaterialParam::getVectorArray() const { return (float4*)&data[0]; }
const float4x4* MaterialParam::getMatrixArray() const { return (float4x4*)&data[0]; }
int MaterialParam::getTexture() const { return (int&)data[0]; }

void MaterialParam::copy(void *dst) const
{
    data.copy_to((char*)dst);
}

} // namespace ms
