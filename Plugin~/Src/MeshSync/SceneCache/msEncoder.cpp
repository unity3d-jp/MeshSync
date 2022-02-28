#include "pch.h"
#include "MeshUtils/MeshUtils.h"
#include "msEncoder.h"

#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>
#pragma comment(lib, "libzstd_static.lib")

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

//----------------------------------------------------------------------------------------------------------------------

class ZSTDBufferEncoder : public BufferEncoder
{
public:
    ZSTDBufferEncoder(int cl);
    void encode(RawVector<char>& dst, const RawVector<char>& src) override;
    void decode(RawVector<char>& dst, const RawVector<char>& src) override;

private:
    int m_compression_level;
};

ZSTDBufferEncoder::ZSTDBufferEncoder(int cl)
{
    m_compression_level = mu::clamp(cl, ZSTD_minCLevel(), ZSTD_maxCLevel());
}

void ZSTDBufferEncoder::encode(RawVector<char>& dst, const RawVector<char>& src)
{
    size_t size = ZSTD_compressBound(src.size());
    dst.resize_discard(size);
    size_t csize = ZSTD_compress(dst.data(), dst.size(), src.data(), src.size(), m_compression_level);
    dst.resize(csize);
}

void ZSTDBufferEncoder::decode(RawVector<char>& dst, const RawVector<char>& src)
{
    size_t dsize = (size_t)ZSTD_findDecompressedSize(src.data(), src.size());
    dst.resize_discard(dsize);
    dsize = ZSTD_decompress(dst.data(), dst.size(), src.data(), src.size());
    dst.resize(dsize);
}

BufferEncoderPtr CreateZSTDEncoder(int compression_level)
{
    return std::make_shared<ZSTDBufferEncoder>(compression_level);
}


} // namespace ms
