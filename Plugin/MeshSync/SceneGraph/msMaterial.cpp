#include "pch.h"
#include "msMaterial.h"


namespace ms {


#define EachMember(F) F(name) F(type) F(data)

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

MaterialProperty::MaterialProperty() {}

template<class T>
static inline void set_impl(RawVector<char>& dst, const T& v)
{
    dst.resize_discard(sizeof(T));
    (T&)dst[0] = v;
}
template<class T>
static inline void set_impl(RawVector<char>& dst, const T *v, size_t n)
{
    dst.resize_discard(sizeof(T) * n);
    memcpy(dst.data(), v, dst.size());
}

template<> void MaterialProperty::set(const int& v) { type = Type::Int; set_impl(data, v); }
template<> void MaterialProperty::set(const float& v) { type = Type::Float; set_impl(data, v); }
template<> void MaterialProperty::set(const float2& v) { type = Type::Vector; set_impl(data, to_vec4(v)); }
template<> void MaterialProperty::set(const float3& v) { type = Type::Vector; set_impl(data, to_vec4(v)); }
template<> void MaterialProperty::set(const float4& v) { type = Type::Vector; set_impl(data, v); }
template<> void MaterialProperty::set(const float2x2& v) { type = Type::Matrix; set_impl(data, to_mat4x4(v)); }
template<> void MaterialProperty::set(const float3x3& v) { type = Type::Matrix; set_impl(data, to_mat4x4(v)); }
template<> void MaterialProperty::set(const float4x4& v) { type = Type::Matrix; set_impl(data, v); }
template<> void MaterialProperty::set(const TexturePtr& v)
{
    type = Type::Texture;
    TextureRecord rec{ v ? v->id : InvalidID };
    set_impl(data, rec);
}
template<> void MaterialProperty::set(const TextureRecord& v)
{
    type = Type::Texture;
    set_impl(data, v);
}

template<> void MaterialProperty::set(const float *v, size_t n)
{
    type = Type::Float;
    set_impl(data, v, n);
}
template<> void MaterialProperty::set(const float2 *v, size_t n)
{
    type = Type::Vector;
    data.resize_discard(sizeof(float4) * n);
    auto *dst = (float4*)data.data();
    for (size_t i = 0; i < n; ++i)
        dst[i] = to_vec4(v[i]);
}
template<> void MaterialProperty::set(const float3 *v, size_t n)
{
    type = Type::Vector;
    data.resize_discard(sizeof(float4) * n);
    auto *dst = (float4*)data.data();
    for (size_t i = 0; i < n; ++i)
        dst[i] = to_vec4(v[i]);
}
template<> void MaterialProperty::set(const float4 *v, size_t n)
{
    type = Type::Vector;
    set_impl(data, v, n);
}
template<> void MaterialProperty::set(const float2x2 *v, size_t n)
{
    type = Type::Matrix;
    data.resize_discard(sizeof(float4x4) * n);
    auto *dst = (float4x4*)data.data();
    for (size_t i = 0; i < n; ++i)
        dst[i] = to_mat4x4(v[i]);
}
template<> void MaterialProperty::set(const float3x3 *v, size_t n)
{
    type = Type::Matrix;
    data.resize_discard(sizeof(float4x4) * n);
    auto *dst = (float4x4*)data.data();
    for (size_t i = 0; i < n; ++i)
        dst[i] = to_mat4x4(v[i]);
}
template<> void MaterialProperty::set(const float4x4 *v, size_t n)
{
    type = Type::Matrix;
    set_impl(data, v, n);
}

size_t MaterialProperty::getArrayLength() const
{
    switch (type) {
    case Type::Unknown: return 0;
    case Type::Int:
    case Type::Float: return (int)data.size() / sizeof(int);
    case Type::Vector: return (int)data.size() / sizeof(float4);
    case Type::Matrix: return (int)data.size() / sizeof(float4x4);
    case Type::Texture: return (int)data.size() / sizeof(TextureRecord);
    default: return 1;
    }
}

template<> int& MaterialProperty::get() const { return (int&)data[0]; }
template<> float& MaterialProperty::get() const { return (float&)data[0]; }
template<> float4& MaterialProperty::get() const { return (float4&)data[0]; }
template<> float4x4& MaterialProperty::get() const { return (float4x4&)data[0]; }
template<> MaterialProperty::TextureRecord& MaterialProperty::get() const { return (TextureRecord&)data[0]; }
template<> const float* MaterialProperty::getArray() const { return (float*)&data[0]; }
template<> const float4* MaterialProperty::getArray() const { return (float4*)&data[0]; }
template<> const float4x4* MaterialProperty::getArray() const { return (float4x4*)&data[0]; }

void MaterialProperty::copy(void* dst)
{
    data.copy_to((char*)dst);
}


#define EachMember(F) F(name) F(value)

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



std::shared_ptr<Material> Material::create(std::istream& is)
{
    return std::static_pointer_cast<Material>(Asset::create(is));
}

Material::Material() {}
Material::~Material() {}

#define EachMember(F) F(index) F(shader) F(properties) F(keywords)

AssetType Material::getAssetType() const
{
    return AssetType::Material;
}

void Material::serialize(std::ostream& os) const
{
    super::serialize(os);
    EachMember(msWrite);
}
void Material::deserialize(std::istream& is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void Material::clear()
{
    super::clear();
    index = 0;
    shader.clear();
    properties.clear();
    keywords.clear();
}

uint64_t Material::hash() const
{
    return checksum();
}

uint64_t Material::checksum() const
{
    uint64_t ret = super::checksum();
    ret += csum(index);
    ret += csum(shader);
    ret += csum(properties);
    ret += csum(keywords);
    return ret;
}
#undef EachMember

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

int Material::getPropertyCount() const
{
    return (int)properties.size();
}
MaterialProperty* Material::getProperty(int i)
{
    return &properties[i];
}
MaterialProperty* Material::findProperty(const char *n)
{
    auto it = std::find_if(properties.begin(), properties.end(), [n](const MaterialProperty& v) { return v.name == n; });
    return it != properties.end() ? &(*it) : nullptr;
}
const MaterialProperty* Material::getProperty(int i) const
{
    return const_cast<Material*>(this)->getProperty(i);
}
const MaterialProperty* Material::findProperty(const char *n) const
{
    return const_cast<Material*>(this)->findProperty(n);
}

void Material::addProperty(MaterialProperty v)
{
    if (auto *p = findProperty(v.name.c_str()))
        *p = v;
    else
        properties.push_back(v);
}

void Material::eraseProperty(const char *n)
{
    auto it = std::find_if(properties.begin(), properties.end(), [n](const MaterialProperty& v) { return v.name == n; });
    if (it != properties.end())
        properties.erase(it);
}

int Material::getKeywordCount() const
{
    return (int)keywords.size();
}
MaterialKeyword* Material::getKeyword(int i)
{
    return &keywords[i];
}
MaterialKeyword* Material::findKeyword(const char *n)
{
    auto it = std::find_if(keywords.begin(), keywords.end(), [n](const MaterialKeyword& v) { return v.name == n; });
    return it != keywords.end() ? &(*it) : nullptr;
}
const MaterialKeyword* Material::getKeyword(int i) const
{
    return const_cast<Material*>(this)->getKeyword(i);
}
const MaterialKeyword* Material::findKeyword(const char *n) const
{
    return const_cast<Material*>(this)->findKeyword(n);
}
void Material::addKeyword(MaterialKeyword v)
{
    if (auto *p = findKeyword(v.name.c_str()))
        *p = v;
    else
        keywords.push_back(v);
}
void Material::eraseKeyword(const char *n)
{
    auto it = std::find_if(keywords.begin(), keywords.end(), [n](const MaterialKeyword& v) { return v.name == n; });
    if (it != keywords.end())
        keywords.erase(it);
}

} // namespace ms
