#include "pch.h"
#include "msAsset.h"
#include "msSceneGraphImpl.h"
#include "msMisc.h"
#include "msAnimation.h"
#include "msTexture.h"
#include "msMaterial.h"

namespace ms {

// Asset

#define EachMember(F) F(name) F(id) 

std::shared_ptr<Asset> Asset::create(std::istream& is)
{
    int type;
    read(is, type);

    std::shared_ptr<Asset> ret;
    switch ((AssetType)type) {
    case AssetType::File: ret = FileAsset::create(); break;
    case AssetType::Animation: ret = AnimationClip::create(); break;
    case AssetType::Texture: ret = Texture::create(); break;
    case AssetType::Material: ret = Material::create(); break;
    default:
        throw std::runtime_error("Asset::create() failed");
        break;
    }
    if (ret)
        ret->deserialize(is);
    return ret;
}

Asset::~Asset()
{
}

uint32_t Asset::getSerializeSize() const
{
    uint32_t ret = 0;
    EachMember(msSize);
    return ret;
}

void Asset::serialize(std::ostream& os) const
{
    int type = (int)getAssetType();
    write(os, type);
    EachMember(msWrite);
}

void Asset::deserialize(std::istream& is)
{
    // type is consumed by create()
    EachMember(msRead);
}

void Asset::clear()
{
    name.clear();
    id = InvalidID;
}

uint64_t Asset::hash() const
{
    return 0;
}

uint64_t Asset::checksum() const
{
    return 0;
}
#undef EachMember

Identifier Asset::getIdentifier() const
{
    return { name, id };
}

bool Asset::identify(const Identifier & v) const
{
    return id != InvalidID ? id == v.id : name == v.name;
}


// FileAsset
#define EachMember(F) F(data)

std::shared_ptr<FileAsset> FileAsset::create(std::istream & is)
{
    return std::static_pointer_cast<FileAsset>(super::create(is));
}

AssetType FileAsset::getAssetType() const
{
    return AssetType::File;
}

uint32_t FileAsset::getSerializeSize() const
{
    uint32_t ret = super::getSerializeSize();
    EachMember(msSize);
    return ret;
}

void FileAsset::serialize(std::ostream& os) const
{
    super::serialize(os);
    EachMember(msWrite);
}

void FileAsset::deserialize(std::istream& is)
{
    super::deserialize(is);
    EachMember(msRead);
}

void FileAsset::clear()
{
    super::clear();
    vclear(data);
}

uint64_t FileAsset::hash() const
{
    return vhash(data);
}

uint64_t FileAsset::checksum() const
{
    uint64_t ret = super::checksum();
    ret += csum(data);
    return ret;
}

bool FileAsset::readFromFile(const char *path)
{
    if (!path)
        return false;
    return FileToByteArray(path, data);
}

bool FileAsset::writeToFile(const char *path) const
{
    if (!path)
        return false;
    return ByteArrayToFile(path, data);
}

#undef EachMember

} // namespace ms
