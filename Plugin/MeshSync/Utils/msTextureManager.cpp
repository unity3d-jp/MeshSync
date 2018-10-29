#include "pch.h"
#include "msTextureManager.h"

namespace ms {

TextureManager::TextureManager()
{
    // random seed
    std::uniform_int_distribution<> d(0, 0x70000000);
    std::mt19937 r;
    r.seed(std::random_device()());
    m_id_seed = d(r);
}

TextureManager::~TextureManager()
{
}

void TextureManager::clear()
{
    m_records.clear();
    m_dirty_list.clear();
}

int TextureManager::find(const char *name) const
{
    auto it = m_records.find(name);
    if (it != m_records.end())
        return it->second.texture->id;
    return -1;
}

int TextureManager::findOrCreate(const char *name, int width, int height, const void *data, size_t size, TextureFormat format)
{
    auto& rec = m_records[name];

    auto hash = genHash(data, size);
    if (!rec.texture || rec.hash != hash) {
        rec.hash = hash;
        rec.texture = Texture::create();
        auto& tex = rec.texture;
        tex->id = ++m_id_seed;
        tex->width = width;
        tex->height = height;
        tex->format = format;
        tex->data.assign((const char*)data, (const char*)data + size);
    }
    return rec.texture->id;
}

int TextureManager::findOrLoad(const char *path, TextureType type)
{
    auto& rec = m_records[path];

    auto mtime = getMTime(path);
    if (!rec.texture || rec.mtime != mtime) {
        rec.mtime = mtime;
        rec.texture = Texture::create();
        auto& tex = rec.texture;
        auto& data = tex->data;
        if (ms::FileToByteArray(path, data)) {
            tex->id = ++m_id_seed;
            tex->name = mu::GetFilename(path);
            tex->format = ms::TextureFormat::RawFile;
            tex->type = type;
            m_dirty_list.push_back(tex);
        }
        else {
            tex->id = -1;
        }
    }
    return rec.texture->id;
}

std::vector<TexturePtr> TextureManager::getDirtyList()
{
    auto ret = m_dirty_list;
    m_dirty_list.clear();
    return ret;
}

Poco::Timestamp TextureManager::getMTime(const char *path)
{
    Poco::File f(path);
    if (f.exists())
        return f.getLastModified();
    else
        return Poco::Timestamp::TIMEVAL_MIN;
}

uint64_t TextureManager::genHash(const void *data_, size_t size)
{
    // todo: optimize
    uint64_t ret = 0;
    auto data = (const uint32_t*)data_;
    size_t count = size / sizeof(int);
    for (size_t i = 0; i < count; ++i) {
        ret += data[i];
    }
    return ret;
}

} // namespace ms
