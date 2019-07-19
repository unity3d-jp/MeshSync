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

    struct TextureRecord
    {
        int id = InvalidID;
        int has_scale_offset = 0;
        float2 scale = float2::one();
        float2 offset = float2::zero();
        TextureRecord(int i)
            : id(i), has_scale_offset(false) {}
        TextureRecord(int i, const float2& s, const float2& o)
            : id(i), has_scale_offset(true), scale(s), offset(o) {}
    };

    std::string name;
    Type type = Type::Unknown;
    RawVector<char> data;

public:
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    uint64_t checksum() const;
    bool operator==(const MaterialProperty& v) const;
    bool operator!=(const MaterialProperty& v) const;

    MaterialProperty();

    // T accepts int, float, float{2,3,4, 2x2, 3x3, 4x4} and TexturePtr/TextureRecord
    // note: float{2,3} are converted to float4 and float{2x2,3x3} are converted to float4x4 internally
    template<class T> MaterialProperty(const char *n, const T& v) : name(n) { set<T>(v); }
    template<class T> MaterialProperty(const char *n, const T *v, size_t c) : name(n) { set<T>(v, c); }
    template<class T> void set(const T& v);
    template<class T> void set(const T *v, size_t n);

    size_t getArrayLength() const;
    // T accepts int, float, float4, float4x4 and TextureRecord.
    // note: float{2,3,2x2,3x3} are not accepted because these were converted to float4 or float4x4.
    //       also, TexturePtr is not accepted because it was converted to TextureRecord.
    template<class T> T& get() const;
    template<class T> T* getArray() const;

    void copy(void *dst);
};
msSerializable(MaterialProperty);


class MaterialKeyword
{
public:
    std::string name;
    bool value = false;

public:
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    uint64_t checksum() const;
    bool operator==(const MaterialKeyword& v) const;
    bool operator!=(const MaterialKeyword& v) const;

    MaterialKeyword();
    MaterialKeyword(const char *n, bool v);
};
msSerializable(MaterialKeyword);


class Material : public Asset
{
using super = Asset;
public:
    using TextureRecord = MaterialProperty::TextureRecord;

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
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    uint64_t checksum() const override;
    bool operator==(const Material& v) const;
    bool operator!=(const Material& v) const;

    int getPropertyCount() const;
    MaterialProperty* getProperty(int i);
    MaterialProperty* findProperty(const char *name);
    const MaterialProperty* getProperty(int i) const;
    const MaterialProperty* findProperty(const char *name) const;
    void addProperty(MaterialProperty v);
    void eraseProperty(const char *name);

    int getKeywordCount() const;
    MaterialKeyword* getKeyword(int i);
    MaterialKeyword* findKeyword(const char *name);
    const MaterialKeyword* getKeyword(int i) const;
    const MaterialKeyword* findKeyword(const char *name) const;
    void addKeyword(MaterialKeyword v);
    void eraseKeyword(const char *name);
};
msSerializable(Material);
msDeclPtr(Material);

} // namespace ms
