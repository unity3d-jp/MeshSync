#pragma once

#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"
#include "msFoundation.h"

namespace ms {

class Texture
{
public:
    enum class TextureType
    {
        Default,
        NormalMap,
    };

    int id = 0;
    TextureType type = TextureType::Default;
    std::string filename;
    RawVector<char> data;

protected:
    Texture();
    ~Texture();
public:
    msDefinePool(Texture);
    static std::shared_ptr<Texture> create(std::istream& is);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
};
msHasSerializer(Texture);
using TexturePtr = std::shared_ptr<Texture>;


class Material
{
public:
    int id = 0;
    std::string name;

    float4 color = float4::one();
    float4 emission = float4::zero();
    float metalic = 0.0f;
    float smoothness = 0.5f;

protected:
    Material();
    ~Material();
public:
    msDefinePool(Material);
    static std::shared_ptr<Material> create(std::istream& is);
    uint32_t getSerializeSize() const;
    void serialize(std::ostream& os) const;
    void deserialize(std::istream& is);
    void clear();
    bool operator==(const Material& v) const;
    bool operator!=(const Material& v) const;
};
msHasSerializer(Material);
using MaterialPtr = std::shared_ptr<Material>;



} // namespace ms
