#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "msEncoder.h"

#ifdef msEnableZSTD
#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>
#pragma comment(lib, "libzstd_static.lib")
#endif

namespace ms {

BufferEncoder::~BufferEncoder()
{
}


class PlainBufferEncoder : public BufferEncoder
{
public:
    void encode(RawVector<char>& dst, const RawVector<char>& src) override;
    void decode(RawVector<char>& dst, const RawVector<char>& src) override;
};

void PlainBufferEncoder::encode(RawVector<char>& dst, const RawVector<char>& src)
{
    dst = src;
}

void PlainBufferEncoder::decode(RawVector<char>& dst, const RawVector<char>& src)
{
    dst = src;
}

BufferEncoderPtr CreatePlainEncoder() { return std::make_shared<PlainBufferEncoder>(); }


#ifdef msEnableZSTD

class ZSTDBufferEncoder : public BufferEncoder
{
public:
    void encode(RawVector<char>& dst, const RawVector<char>& src) override;
    void decode(RawVector<char>& dst, const RawVector<char>& src) override;
};

void ZSTDBufferEncoder::encode(RawVector<char>& dst, const RawVector<char>& src)
{
    size_t size = ZSTD_compressBound(src.size());
    dst.resize_discard(size);
    size_t csize = ZSTD_compress(dst.data(), dst.size(), src.data(), src.size(), ZSTD_CLEVEL_DEFAULT);
    dst.resize(csize);
}

void ZSTDBufferEncoder::decode(RawVector<char>& dst, const RawVector<char>& src)
{
    size_t dsize = ZSTD_findDecompressedSize(src.data(), src.size());
    dst.resize(dsize);
    dsize = ZSTD_decompress(dst.data(), dst.size(), src.data(), src.size());
    dst.resize(dsize);
}

BufferEncoderPtr CreateZSTDEncoder() { return std::make_shared<ZSTDBufferEncoder>(); }

#else
BufferEncoderPtr CreateZSTDEncoder() { return nullptr; }
#endif

} // namespace ms
