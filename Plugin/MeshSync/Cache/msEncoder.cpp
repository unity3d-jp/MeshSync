#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "msEncoder.h"

#ifdef msEnableZSTD
#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>
#pragma comment(lib, "libzstd_static.lib")
#endif

namespace ms {

Encoder::~Encoder()
{
}


class PlainEncoder : public Encoder
{
public:
    void encode(RawVector<char>& dst, const RawVector<char>& src) override;
    void decode(RawVector<char>& dst, const RawVector<char>& src) override;
};

void PlainEncoder::encode(RawVector<char>& dst, const RawVector<char>& src)
{
    dst = src;
}

void PlainEncoder::decode(RawVector<char>& dst, const RawVector<char>& src)
{
    dst = src;
}

EncoderPtr CreatePlainEncoder() { return std::make_shared<PlainEncoder>(); }


#ifdef msEnableZSTD

class ZSTDEncoder : public Encoder
{
public:
    void encode(RawVector<char>& dst, const RawVector<char>& src) override;
    void decode(RawVector<char>& dst, const RawVector<char>& src) override;
};

void ZSTDEncoder::encode(RawVector<char>& dst, const RawVector<char>& src)
{
    size_t size = ZSTD_compressBound(src.size());
    dst.resize_discard(size);
    size_t csize = ZSTD_compress(dst.data(), dst.size(), src.data(), src.size(), ZSTD_CLEVEL_DEFAULT);
    dst.resize(csize);
}

void ZSTDEncoder::decode(RawVector<char>& dst, const RawVector<char>& src)
{
    size_t dsize = ZSTD_findDecompressedSize(src.data(), src.size());
    dst.resize(dsize);
    dsize = ZSTD_decompress(dst.data(), dst.size(), src.data(), src.size());
    dst.resize(dsize);
}

EncoderPtr CreateZSTDEncoder() { return std::make_shared<ZSTDEncoder>(); }

#else
EncoderPtr CreateZSTDEncoder() { return nullptr; }
#endif

} // namespace ms
