#pragma once

#include "msTexture.h"

namespace ms {

class MaterialProperty
{
public:
    enum class Type {
        Unknown,
        Int,
        Float,
        Vector,
        Matrix,
        Texture,
    };

    std::string name;
    Type type = Type::Unknown;
    RawVector<char> data;

public:
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    uint64_t checksum() const;
    bool operator==(const MaterialProperty& v) const;
    bool operator!=(const MaterialProperty& v) const;

    MaterialProperty();
    MaterialProperty(const char *name);
    MaterialProperty(const char *name, int v);
    MaterialProperty(const char *name, float v);
    MaterialProperty(const char *name, const float4& v);
    MaterialProperty(const char *name, const float4x4& v);
    MaterialProperty(const char *name, const float *v, int count);
    MaterialProperty(const char *name, const float4 *v, int count);
    MaterialProperty(const char *name, const float4x4 *v, int count);
    MaterialProperty(const char *name, TexturePtr v);
    MaterialProperty(const char *name, Texture *v);

    void setInt(int v);
    void setFloat(float v);
    void setFloat2(const float2& v);
    void setFloat3(const float3& v);
    void setFloat4(const float4& v);
    void setFloat2x2(const float2x2& v);
    void setFloat3x3(const float3x3& v);
    void setFloat4x4(const float4x4& v);
    void setFloat(const float *v, int count);
    void setFloat2(const float2 *v, int count);
    void setFloat3(const float3 *v, int count);
    void setFloat4(const float4 *v, int count);
    void setFloat2x2(const float2x2 *v, int count);
    void setFloat3x3(const float3x3 *v, int count);
    void setFloat4x4(const float4x4 *v, int count);
    void setTexture(int v);
    void setTexture(TexturePtr v);
    void setTexture(Texture *v);

    int getArraySize() const;
    int getInt() const;
    float getFloat() const;
    float4 getFloat4() const;
    float4x4 getFloat4x4() const;
    const float* getFloatArray() const;
    const float4* getFloat4Array() const;
    const float4x4* getFloat4x4Array() const;
    int getTexture() const;
    void copy(void *dst) const;
};
msHasSerializer(MaterialProperty);


class MaterialKeyword
{
public:
    std::string name;
    bool value = false;

public:
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    uint64_t checksum() const;
    bool operator==(const MaterialKeyword& v) const;
    bool operator!=(const MaterialKeyword& v) const;

    MaterialKeyword();
    MaterialKeyword(const char *n, bool v);
};
msHasSerializer(MaterialKeyword);


class Material : public Asset
{
using super = Asset;
public:
    int index = 0;
    std::string shader;
    std::vector<MaterialProperty> properties;
    std::vector<MaterialKeyword> keywords;

protected:
    Material();
    ~Material() override;
public:
    msDefinePool(Material);
    static std::shared_ptr<Material> create(std::istream& is);

    AssetType getAssetType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    uint64_t checksum() const override;
    bool operator==(const Material& v) const;
    bool operator!=(const Material& v) const;

    int getParamCount() const;
    MaterialProperty* getParam(int i);
    MaterialProperty* findParam(const char *name);
    const MaterialProperty* getParam(int i) const;
    const MaterialProperty* findParam(const char *name) const;
    void addParam(MaterialProperty v);
    void eraseParam(const char *name);

    int getKeywordCount() const;
    MaterialKeyword* getKeyword(int i);
    MaterialKeyword* findKeyword(const char *name);
    const MaterialKeyword* getKeyword(int i) const;
    const MaterialKeyword* findKeyword(const char *name) const;
    void addKeyword(MaterialKeyword v);
    void eraseKeyword(const char *name);
};
msHasSerializer(Material);
using MaterialPtr = std::shared_ptr<Material>;

} // namespace ms
