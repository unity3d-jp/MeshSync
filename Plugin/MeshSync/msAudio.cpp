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

static inline int SizeOf(AudioFormat f)
{
    int ret = 0;
    switch (f) {
    case AudioFormat::U8: ret = 1; break;
    case AudioFormat::S16: ret = 2; break;
    case AudioFormat::S24: ret = 3; break;
    case AudioFormat::F32: ret = 4; break;
    default: break;
    }
    return ret;
}

void* Audio::allocate(int num_samples)
{
    data.resize(channels * SizeOf(format) * num_samples);
    return data.data();
}

size_t Audio::getNumSamples() const
{
    return data.size() / (channels * SizeOf(format));
}

double Audio::getDuration() const
{
    return (double)getNumSamples() / frequency;
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

bool Audio::convertSamplesToFloat(float *dst)
{
    size_t n = data.size() / SizeOf(format);
    if (format == AudioFormat::U8)
        U8NToF32(dst, (unorm8n*)data.data(), n);
    else if (format == AudioFormat::S16)
        S16ToF32(dst, (snorm16*)data.data(), n);
    else if (format == AudioFormat::S24)
        S24ToF32(dst, (snorm24*)data.data(), n);
    else if (format == AudioFormat::F32)
        data.copy_to((char*)dst);
    else
        return false;
    return true;
}

} // namespace ms
