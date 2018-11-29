#pragma once

#include "msAsset.h"

namespace ms {

enum class AudioFormat
{
    Unknown = 0,
    U8,
    S16,
    S24,
    S32,
    F32,
    RawFile = 100,
};


class Audio : public Asset
{
using super = Asset;
public:
    // if format is RawFile, Asset::name must be file name

    AudioFormat format = AudioFormat::Unknown;
    int frequency = 0;
    int channels = 0;
    RawVector<char> data;

protected:
    Audio();
    ~Audio() override;
public:
    msDefinePool(Audio);
    static std::shared_ptr<Audio> create(std::istream& is);

    AssetType getAssetType() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    uint64_t checksum() const override;

    // allocate num_samples * channels * size_of_format bytes
    void* allocate(int num_samples);
    size_t getSampleLength() const;
    double getDuration() const;
    bool readFromFile(const char *path);
    bool writeToFile(const char *path) const;
    bool exportAsWave(const char *path) const;

    // length of dst must be frequency * channels
    bool convertSamplesToFloat(float *dst);
};
msSerializable(Audio);
msDeclPtr(Audio);

} // namespace ms
