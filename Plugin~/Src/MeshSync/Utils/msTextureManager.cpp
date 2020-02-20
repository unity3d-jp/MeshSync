#include "pch.h"
#include "msTextureManager.h"
#include "msMisc.h"

#ifndef msRuntime
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
    waitTasks();
}

bool TextureManager::empty() const
{
    return m_records.empty();
}

void TextureManager::clear()
{
    waitTasks();
    m_records.clear();
}

bool TextureManager::erase(const std::string& name)
{
    return m_records.erase(name) != 0;
}

int TextureManager::find(const std::string& name) const
{
    auto it = m_records.find(name);
    if (it != m_records.end())
        return it->second.texture->id;
    return InvalidID;
}

int TextureManager::addImage(const std::string& name, int width, int height, const void *data, size_t size, TextureFormat format)
{
    auto& rec = lockAndGet(name);
    int id = rec.texture ?
        rec.texture->id :
        (data && size ? genID() : -1);

    // not worth to make tasks
    auto checksum = SumInt32(data, size);
    if (!rec.texture || rec.checksum != checksum) {
        rec.checksum = checksum;
        rec.texture = Texture::create();
        auto& tex = rec.texture;
        tex->id = id;
        tex->name = name;
        tex->format = format;
        tex->width = width;
        tex->height = height;
        tex->data.assign((const char*)data, (const char*)data + size);
        rec.dirty = true;
    }
    if (m_always_mark_dirty)
        rec.dirty = true;
    return id;
}

int TextureManager::addFile(const std::string& path, TextureType type)
{
    if (path.empty())
        return InvalidID;

    auto& rec = lockAndGet(path);
    int id = InvalidID;
    if (rec.texture) {
        id = rec.texture->id;
    }
    else {
        if (!FileExists(path.c_str()))
            return InvalidID;
        id = genID();
    }

    rec.waitTask();
    rec.task = std::async(std::launch::async, [this, path, type, &rec, id]() {
        auto mtime = FileMTime(path.c_str());
        if (!rec.texture || rec.mtime != mtime) {
            if (!rec.texture)
                rec.texture = Texture::create();
            rec.mtime = mtime;
            auto& tex = rec.texture;
            if (FileToByteArray(path.c_str(), rec.texture->data)) {
                tex->id = id;
                tex->name = mu::GetFilename(path.c_str());
                tex->format = ms::TextureFormat::RawFile;
                tex->type = type;
                rec.dirty = true;
            }
        }
        if (m_always_mark_dirty)
            rec.dirty = true;
    });
    return id;
}

int TextureManager::add(TexturePtr tex)
{
    if (!tex)
        return InvalidID;

    auto& rec = lockAndGet(tex->name);
    auto checksum = SumInt32(tex->data.data(), tex->data.size());
    if (!rec.texture || rec.checksum != checksum) {
        rec.checksum = checksum;
        rec.texture = tex;
        rec.dirty = true;
    }
    return tex->id;
}

std::vector<TexturePtr> TextureManager::getAllTextures()
{
    waitTasks();

    std::vector<TexturePtr> ret;
    for (auto& kvp : m_records) {
        auto& rec = kvp.second;
        rec.waitTask();
        if (rec.texture)
            ret.push_back(rec.texture);
    }
    return ret;
}

std::vector<TexturePtr> TextureManager::getDirtyTextures()
{
    waitTasks();

    std::vector<TexturePtr> ret;
    for (auto& kvp : m_records) {
        auto& rec = kvp.second;
        rec.waitTask();
        if (rec.dirty && rec.texture)
            ret.push_back(rec.texture);
    }
    return ret;
}

void TextureManager::makeDirtyAll()
{
    for (auto& kvp : m_records) {
        kvp.second.dirty = true;
    }
}

void TextureManager::clearDirtyFlags()
{
    for (auto& kvp : m_records) {
        kvp.second.dirty = false;
    }
}

void TextureManager::setAlwaysMarkDirty(bool v)
{
    m_always_mark_dirty = v;
}

int TextureManager::genID()
{
    return ++m_id_seed;
}

void TextureManager::waitTasks()
{
    for (auto& p : m_records)
        p.second.waitTask();
}

TextureManager::Record& TextureManager::lockAndGet(const std::string &path)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_records[path];
}

void TextureManager::Record::waitTask()
{
    if (task.valid()) {
        task.wait();
        task = {};
    }
}

} // namespace ms
#endif // msRuntime
