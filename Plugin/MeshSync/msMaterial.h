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
bool FileToByteArray(const char *path, RawVector<char> &out);
bool ByteArrayToFile(const char *path, const RawVector<char> &data);
bool FileExists(const char *path);
uint64_t FileMTime(const char *path);

class Texture
{
public:
    int id = InvalidID;
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
    uint64_t hash() const;
    uint64_t checksum() const;

    void setData(const void *src);
    void getData(void *dst) const;
    bool writeToFile(const char *path);
};
msHasSerializer(Texture);
using TexturePtr = std::shared_ptr<Texture>;


struct MaterialDataFlags
{
    uint32_t has_color : 1;
    uint32_t has_color_map : 1;
    uint32_t has_emission : 1;
    uint32_t has_emission_map : 1;
    uint32_t has_metallic : 1;
    uint32_t has_smoothness : 1;
    uint32_t has_metallic_map : 1;
    uint32_t has_normal_map : 1;
};

class Material
{
public:
    int id = InvalidID;
    std::string name;
    MaterialDataFlags flags = { 0 };

protected:
    float4 color = float4::one();
    float4 emission = float4::zero();
    float metalic = 0.0f;
    float smoothness = 0.5f;

    // texture ids
    int color_map = InvalidID;
    int metallic_map = InvalidID;
    int emission_map = InvalidID;
    int normal_map = InvalidID;

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
    uint64_t checksum() const;
    bool operator==(const Material& v) const;
    bool operator!=(const Material& v) const;

    void    setColor(float4 v);
    float4  getColor() const;
    void    setEmission(float4 v);
    float4  getEmission() const;
    void    setMetallic(float v);
    float   getMetallic() const;
    void    setSmoothness(float v);
    float   getSmoothness() const;

    void    setColorMap(int v);
    int     getColorMap() const;
    void    setEmissionMap(int v);
    int     getEmissionMap() const;
    void    setMetallicMap(int v);
    int     getMetallicMap() const;
    void    setNormalMap(int v);
    int     getNormalMap() const;
};
msHasSerializer(Material);
using MaterialPtr = std::shared_ptr<Material>;



} // namespace ms
