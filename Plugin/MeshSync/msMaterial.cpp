#include "pch.h"
#include "msMaterial.h"
#include "msSceneGraphImpl.h"


namespace ms {




#define EachMember(F) F(name) F(type) F(data)

uint32_t MaterialProperty::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}

void MaterialProperty::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}

void MaterialProperty::deserialize(std::istream& is)
{
    EachMember(msRead);
}
uint64_t MaterialProperty::checksum() const
{
    return mu::SumInt32(data.data(), data.size());
}
bool MaterialProperty::operator==(const MaterialProperty& v) const
{
    return
        name == v.name &&
        type == v.type &&
        data == v.data;
}
bool MaterialProperty::operator!=(const MaterialProperty& v) const
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
MaterialProperty::MaterialProperty() {}
MaterialProperty::MaterialProperty(const char *n, int v) : name(n), type(Type::Int) { set_impl(data, v); }
MaterialProperty::MaterialProperty(const char *n, float v) : name(n), type(Type::Float) { set_impl(data, v); }
MaterialProperty::MaterialProperty(const char *n, const float4& v) : name(n), type(Type::Vector) { set_impl(data, v); }
MaterialProperty::MaterialProperty(const char *n, const float4x4& v) : name(n), type(Type::Matrix) { set_impl(data, v); }
MaterialProperty::MaterialProperty(const char *n, const float *v, int c) : name(n), type(Type::FloatArray) { set_impl(data, v, c); }
MaterialProperty::MaterialProperty(const char *n, const float4 *v, int c) : name(n), type(Type::VectorArray) { set_impl(data, v, c); }
MaterialProperty::MaterialProperty(const char *n, const float4x4 *v, int c) : name(n), type(Type::MatrixArray) { set_impl(data, v, c); }
MaterialProperty::MaterialProperty(const char *n, TexturePtr v) : name(n), type(Type::Texture) { set_impl(data, v ? v->id : InvalidID); }
MaterialProperty::MaterialProperty(const char *n, Texture *v) : name(n), type(Type::Texture) { set_impl(data, v ? v->id : InvalidID); }

int MaterialProperty::getArraySize() const
{
    switch (type) {
    case Type::Unknown: return 0;
    case Type::FloatArray: return (int)data.size() / sizeof(float);
    case Type::VectorArray: return (int)data.size() / sizeof(float4);
    case Type::MatrixArray: return (int)data.size() / sizeof(float4x4);
    default: return 1;
    }
}

int MaterialProperty::getInt() const { return (int&)data[0]; }
float MaterialProperty::getFloat() const { return (float&)data[0]; }
float4 MaterialProperty::getVector() const { return (float4&)data[0]; }
float4x4 MaterialProperty::getMatrix() const { return (float4x4&)data[0]; }
const float* MaterialProperty::getFloatArray() const { return (float*)&data[0]; }
const float4* MaterialProperty::getVectorArray() const { return (float4*)&data[0]; }
const float4x4* MaterialProperty::getMatrixArray() const { return (float4x4*)&data[0]; }
int MaterialProperty::getTexture() const { return (int&)data[0]; }

void MaterialProperty::copy(void *dst) const
{
    data.copy_to((char*)dst);
}


#define EachMember(F) F(name) F(value)

uint32_t MaterialKeyword::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}
void MaterialKeyword::serialize(std::ostream& os) const
{
    EachMember(msWrite);
}
void MaterialKeyword::deserialize(std::istream& is)
{
    EachMember(msRead);
}
#undef EachMember

uint64_t MaterialKeyword::checksum() const
{
    return uint64_t(value);
}
bool MaterialKeyword::operator==(const MaterialKeyword& v) const
{
    return name == v.name && value == v.value;
}
bool MaterialKeyword::operator!=(const MaterialKeyword& v) const
{
    return !(*this == v);
}

MaterialKeyword::MaterialKeyword()
{
}

MaterialKeyword::MaterialKeyword(const char *n, bool v)
    : name(n), value(v)
{}



std::shared_ptr<Material> Material::create(std::istream & is)
{
    auto ret = Pool<Material>::instance().pull();
    ret->deserialize(is);
    return make_shared_ptr(ret);
}

Material::Material() {}
Material::~Material() {}

#define EachMember(F) F(id) F(name) F(shader) F(properties) F(keywords)

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
    properties.clear();
}

uint64_t Material::checksum() const
{
    uint64_t ret = 0;
    ret += csum(shader);
    ret += csum(properties);
    return ret;
}

bool Material::operator==(const Material& v) const
{
    return
        name == v.name &&
        shader == v.shader &&
        properties == v.properties &&
        keywords == v.keywords;
}
bool Material::operator!=(const Material& v) const
{
    return !(*this == v);
}

int Material::getParamCount() const
{
    return (int)properties.size();
}
MaterialProperty* Material::getParam(int i)
{
    return &properties[i];
}
MaterialProperty* Material::findParam(const char *n)
{
    auto it = std::find_if(properties.begin(), properties.end(), [n](const MaterialProperty& v) { return v.name == n; });
    return it != properties.end() ? &(*it) : nullptr;
}
const MaterialProperty* Material::getParam(int i) const
{
    return const_cast<Material*>(this)->getParam(i);
}
const MaterialProperty* Material::findParam(const char *n) const
{
    return const_cast<Material*>(this)->findParam(n);
}

void Material::addParam(MaterialProperty v)
{
    if (auto *p = findParam(v.name.c_str()))
        *p = v;
    else
        properties.push_back(v);
}

void Material::eraseParam(const char *n)
{
    auto it = std::find_if(properties.begin(), properties.end(), [n](const MaterialProperty& v) { return v.name == n; });
    if (it != properties.end())
        properties.erase(it);
}

} // namespace ms
