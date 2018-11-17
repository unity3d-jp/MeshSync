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

template<class T> struct GetTextureFormat;
template<> struct GetTextureFormat<unorm8>  { static const TextureFormat result = ms::TextureFormat::Ru8; };
template<> struct GetTextureFormat<unorm8x2>{ static const TextureFormat result = ms::TextureFormat::RGu8; };
template<> struct GetTextureFormat<unorm8x3>{ static const TextureFormat result = ms::TextureFormat::RGBu8; };
template<> struct GetTextureFormat<unorm8x4>{ static const TextureFormat result = ms::TextureFormat::RGBAu8; };
template<> struct GetTextureFormat<half>    { static const TextureFormat result = ms::TextureFormat::Rf16; };
template<> struct GetTextureFormat<half2>   { static const TextureFormat result = ms::TextureFormat::RGf16; };
template<> struct GetTextureFormat<half3>   { static const TextureFormat result = ms::TextureFormat::RGBf16; };
template<> struct GetTextureFormat<half4>   { static const TextureFormat result = ms::TextureFormat::RGBAf16; };
template<> struct GetTextureFormat<float>   { static const TextureFormat result = ms::TextureFormat::Rf32; };
template<> struct GetTextureFormat<float2>  { static const TextureFormat result = ms::TextureFormat::RGf32; };
template<> struct GetTextureFormat<float3>  { static const TextureFormat result = ms::TextureFormat::RGBf32; };
template<> struct GetTextureFormat<float4>  { static const TextureFormat result = ms::TextureFormat::RGBAf32; };


// in byte
int GetPixelSize(TextureFormat format);
bool FileToByteArray(const char *path, RawVector<char> &out);
bool ByteArrayToFile(const char *path, const RawVector<char> &data);
bool ByteArrayToFile(const char *path, const char *data, size_t size);
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

class MaterialParam
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
    bool operator==(const MaterialParam& v) const;
    bool operator!=(const MaterialParam& v) const;

    MaterialParam();
    MaterialParam(const char *name, int v);
    MaterialParam(const char *name, float v);
    MaterialParam(const char *name, const float *v, int count);
    MaterialParam(const char *name, const float4& v);
    MaterialParam(const char *name, const float4 *v, int count);
    MaterialParam(const char *name, const float4x4& v);
    MaterialParam(const char *name, const float4x4 *v, int count);
    MaterialParam(const char *name, TexturePtr v);
    MaterialParam(const char *name, Texture *v);

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
msHasSerializer(MaterialParam);

class Material
{
public:
    int id = InvalidID;
    std::string name;
    std::string shader = "Standard";
    std::vector<MaterialParam> params;

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
    MaterialParam* getParam(int i);
    MaterialParam* findParam(const char *name);
    const MaterialParam* getParam(int i) const;
    const MaterialParam* findParam(const char *name) const;
    void addParam(MaterialParam v);
    void eraseParam(const char *name);
};
msHasSerializer(Material);
using MaterialPtr = std::shared_ptr<Material>;

} // namespace ms
