#pragma once
#include "msFoundation.h"

namespace ms {

enum class AssetType
{
    Unknown,
    File,
    Animation,
    Texture,
    Material,
};

class Asset
{
public:
    static std::shared_ptr<Asset> create(std::istream& is);

public:
    std::string name;
    int id = InvalidID;

public:
    virtual ~Asset();
    virtual AssetType getAssetType() const = 0;
    virtual uint32_t getSerializeSize() const;
    virtual void serialize(std::ostream& os) const;
    virtual void deserialize(std::istream& is);
    virtual void clear();
    virtual uint64_t hash() const;
    virtual uint64_t checksum() const;

    Identifier getIdentifier() const;
    bool identify(const Identifier& v) const;

};
msHasSerializer(Asset);
using AssetPtr = std::shared_ptr<Asset>;



// generic file asset
class FileAsset : public Asset
{
using super = Asset;
public:
    // Asset::name must be file name

    RawVector<char> data;

public:
    msDefinePool(FileAsset);
    static std::shared_ptr<FileAsset> create(std::istream& is);

    AssetType getAssetType() const override;
    uint32_t getSerializeSize() const override;
    void serialize(std::ostream& os) const override;
    void deserialize(std::istream& is) override;
    void clear() override;
    uint64_t hash() const override;
    uint64_t checksum() const override;

    bool readFromFile(const char *path);
    bool writeToFile(const char *path) const;
};
msHasSerializer(FileAsset);
using FileAssetPtr = std::shared_ptr<FileAsset>;

} // namespace ms
