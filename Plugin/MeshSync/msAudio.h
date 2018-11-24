#pragma once

#include "msAsset.h"

namespace ms {

enum class AudioFormat
{
    Unknown = 0,
    U8,
    S16,
    S24,
    F32,
    RawFile,
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
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    uint64_t checksum() const override;

    void* allocate(int num_samples);
    size_t getNumSamples() const;
    double getDuration() const;
    bool readFromFile(const char *path);
    bool writeToFile(const char *path) const;

    // length of dst must be frequency * channels
    bool convertSamplesToFloat(float *dst);
};
msHasSerializer(Audio);
using AudioPtr = std::shared_ptr<Audio>;

} // namespace ms
