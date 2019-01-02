#include "pch.h"
#include "msAsset.h"
#include "msAudio.h"
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
    case AudioFormat::S32: ret = 4; break;
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

size_t Audio::getSampleLength() const
{
    return data.size() / SizeOf(format);
}

double Audio::getDuration() const
{
    return (double)getSampleLength() / (frequency * channels);
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

struct WaveHeader
{
    char    RIFFTag[4] = { 'R', 'I', 'F', 'F' };
    int32_t nFileSize = 0;
    char    WAVETag[4] = { 'W', 'A', 'V', 'E' };
    char    fmtTag[4] = { 'f', 'm', 't', ' ' };
    int32_t nFmtSize = 16;
    int16_t shFmtID = 1;
    int16_t shCh = 2;
    int32_t nSampleRate = 48000;
    int32_t nBytePerSec = 96000;
    int16_t shBlockSize = 4;
    int16_t shBitPerSample = 16;
    char    dataTag[4] = { 'd', 'a', 't', 'a' };
    int32_t nBytesData = 0;
};

bool Audio::exportAsWave(const char *path) const
{
    if (format == AudioFormat::RawFile)
        return false;

    std::ofstream os(path, std::ios::binary);
    if (!os)
        return false;

    RawVector<char> tmp_data;

    const RawVector<char> *wdata = &data;
    AudioFormat wfmt = format;
    // if format is f32, convert to s16
    if (format == AudioFormat::F32) {
        wfmt = AudioFormat::S16;
        tmp_data.resize(getSampleLength() * sizeof(snorm16));
        F32ToS16((snorm16*)tmp_data.data(), (const float*)data.data(), getSampleLength());
        wdata = &tmp_data;
    }

    WaveHeader header;
    header.nSampleRate = frequency;
    header.shCh = (int16_t)channels;
    header.shBitPerSample = (int16_t)SizeOf(wfmt) * 8;
    header.nBytePerSec = header.nSampleRate * header.shBitPerSample * header.shCh / 8;
    header.shBlockSize = header.shBitPerSample * header.shCh / 8;
    os.write((char*)&header, sizeof(header));
    os.write(wdata->data(), wdata->size());

    uint32_t total_size = (uint32_t)(wdata->size() + sizeof(WaveHeader));
    uint32_t filesize = total_size - 8;
    uint32_t datasize = total_size - 44;
    os.seekp(4);
    os.write((char*)&filesize, 4);
    os.seekp(40);
    os.write((char*)&datasize, 4);
    return true;
}


bool Audio::convertSamplesToFloat(float *dst)
{
    size_t n = data.size() / SizeOf(format);
    if (n == 0)
        return false; // invalid format

    if (format == AudioFormat::U8)
        U8NToF32(dst, (unorm8n*)data.data(), n);
    else if (format == AudioFormat::S16)
        S16ToF32(dst, (snorm16*)data.data(), n);
    else if (format == AudioFormat::S24)
        S24ToF32(dst, (snorm24*)data.data(), n);
    else if (format == AudioFormat::S32)
        S32ToF32(dst, (snorm32*)data.data(), n);
    else if (format == AudioFormat::F32)
        data.copy_to((char*)dst);
    else
        return false;
    return true;
}

} // namespace ms
