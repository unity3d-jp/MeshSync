#include "pch.h"
#include "msAsset.h"
#include "msAudio.h"
#include "msSceneGraphImpl.h"
#include "msMisc.h"

namespace ms {

#define EachMember(F)  F(format) F(frequency) F(channels) F(data)

Audio::Audio() {}
Audio::~Audio() {}

std::shared_ptr<Audio> Audio::create(std::istream & is)
{
    return std::static_pointer_cast<Audio>(Asset::create(is));
}

AssetType Audio::getAssetType() const
{
    return AssetType::Audio;
}

uint32_t Audio::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(msSize);
    return ret;
}

void Audio::serialize(std::ostream & os) const
{
    super::serialize(os);
    EachMember(msWrite);
}

void Audio::deserialize(std::istream & is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void Audio::clear()
{
    super::clear();
    format = AudioFormat::Unknown;
    frequency = channels = 0;
    vclear(data);
}

uint64_t Audio::hash() const
{
    uint64_t ret = 0;
    ret += vhash(data);
    return ret;
}

uint64_t Audio::checksum() const
{
    uint64_t ret = 0;
    ret += csum((int)format);
    ret += csum(frequency);
    ret += csum(channels);
    ret += csum(data);
    return ret;
}

bool Audio::readFromFile(const char * path)
{
    if (!path)
        return false;
    if (FileToByteArray(path, data)) {
        name = GetFilename(path);
        format = AudioFormat::RawFile;
        return true;
    }
    return false;
}

bool Audio::writeToFile(const char * path) const
{
    if (!path)
        return false;
    return ByteArrayToFile(path, data);
}

template<class T>
static inline void ConvertImpl(float *dst, const RawVector<char>& data)
{
    size_t n = data.size() / sizeof(T);
    auto *src = (T*)data.data();
    for (size_t si = 0; si < n; ++si)
        dst[si] = src[si];
}

bool Audio::convertSamplesToFloat(float *dst)
{
    if (format == AudioFormat::U8)
        ConvertImpl<unorm8n>(dst, data);
    else if (format == AudioFormat::I16)
        ConvertImpl<snorm16>(dst, data);
    else if (format == AudioFormat::F32)
        ConvertImpl<float>(dst, data);
    else
        return false;
    return true;
}

} // namespace ms
