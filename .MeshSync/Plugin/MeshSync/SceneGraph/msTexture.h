#pragma once
#include "msAsset.h"

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

class Texture : public Asset
{
using super = Asset;
public:
    // if format is RawFile, Asset::name must be file name

    TextureType type = TextureType::Default;
    TextureFormat format = TextureFormat::Unknown;
    int width = 0;
    int height = 0;
    RawVector<char> data;

protected:
    Texture();
    ~Texture() override;
public:
    msDefinePool(Texture);
    static std::shared_ptr<Texture> create(std::istream& is);

    AssetType getAssetType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    uint64_t checksum() const override;

    void setData(const void *src);
    void getData(void *dst) const;
    bool readFromFile(const char *path);
    bool writeToFile(const char *path) const;
};
msSerializable(Texture);
msDeclPtr(Texture);

} // namespace ms
