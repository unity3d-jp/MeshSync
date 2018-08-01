#pragma once

#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"
#include "msFoundation.h"

namespace ms {

enum class TextureFormat
{
    Unknown = 0,

    ChannelMask = 0xF,
    TypeMask = 0xF << 4,
    Type_f16 = 0x1 << 4,
    Type_f32 = 0x2 << 4,
    Type_u8  = 0x3 << 4,
    Type_i16 = 0x4 << 4,
    Type_i32 = 0x5 << 4,

    Rf16      = Type_f16 | 1,
    RGf16     = Type_f16 | 2,
    RGBf16    = Type_f16 | 3,
    RGBAf16   = Type_f16 | 4,
    Rf32      = Type_f32 | 1,
    RGf32     = Type_f32 | 2,
    RGBf32    = Type_f32 | 3,
    RGBAf32   = Type_f32 | 4,
    Ru8       = Type_u8  | 1,
    RGu8      = Type_u8  | 2,
    RGBu8     = Type_u8  | 3,
    RGBAu8    = Type_u8  | 4,
    Ri16      = Type_i16 | 1,
    RGi16     = Type_i16 | 2,
    RGBi16    = Type_i16 | 3,
    RGBAi16   = Type_i16 | 4,
    Ri32      = Type_i32 | 1,
    RGi32     = Type_i32 | 2,
    RGBi32    = Type_i32 | 3,
    RGBAi32   = Type_i32 | 4,

    RawFile = 0x10 << 4,
};

enum class TextureType
{
    Default,
    NormalMap,
};

// in byte
int GetPixelSize(TextureFormat format);

class Texture
{
public:
    int id = 0;
    std::string name; // if format is RawFile, this must be file name
    TextureType type = TextureType::Default;
    TextureFormat format = TextureFormat::Unknown;
    int width = 0;
    int height = 0;
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

    void setData(const void *src);
    void getData(void *dst) const;
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
