#pragma once

#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"
#include "msFoundation.h"
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
        FloatArray,
        VectorArray,
        MatrixArray,
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
    MaterialProperty(const char *name, int v);
    MaterialProperty(const char *name, float v);
    MaterialProperty(const char *name, const float *v, int count);
    MaterialProperty(const char *name, const float4& v);
    MaterialProperty(const char *name, const float4 *v, int count);
    MaterialProperty(const char *name, const float4x4& v);
    MaterialProperty(const char *name, const float4x4 *v, int count);
    MaterialProperty(const char *name, TexturePtr v);
    MaterialProperty(const char *name, Texture *v);

    int getArraySize() const;
    int getInt() const;
    float getFloat() const;
    float4 getVector() const;
    float4x4 getMatrix() const;
    const float* getFloatArray() const;
    const float4* getVectorArray() const;
    const float4x4* getMatrixArray() const;
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


class Material
{
public:
    int id = InvalidID;
    std::string name;
    std::string shader = "Standard";
    std::vector<MaterialProperty> properties;
    std::vector<MaterialKeyword> keywords;

protected:
    Material();
    virtual ~Material();
public:
    msDefinePool(Material);
    static std::shared_ptr<Material> create(std::istream& is);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
    uint64_t checksum() const;
    bool operator==(const Material& v) const;
    bool operator!=(const Material& v) const;

    int getParamCount() const;
    MaterialProperty* getParam(int i);
    MaterialProperty* findParam(const char *name);
    const MaterialProperty* getParam(int i) const;
    const MaterialProperty* findParam(const char *name) const;
    void addParam(MaterialProperty v);
    void eraseParam(const char *name);
};
msHasSerializer(Material);
using MaterialPtr = std::shared_ptr<Material>;

} // namespace ms
