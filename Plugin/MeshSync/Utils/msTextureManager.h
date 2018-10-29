#pragma once

#include "../msMaterial.h"

namespace ms {

class TextureManager
{
public:
    struct Record
    {
        TexturePtr texture;
        Poco::Timestamp mtime = Poco::Timestamp::TIMEVAL_MIN;
        uint64_t hash = 0;
    };
    
    TextureManager();
    ~TextureManager();
    void clear();
    int find(const char *name) const;
    int findOrCreate(const char *name, int width, int height, const void *data, size_t size, TextureFormat format);
    int findOrLoad(const char *path, TextureType type);
    std::vector<TexturePtr> getAllTextures();
    std::vector<TexturePtr> getDirtyList();

    static Poco::Timestamp getMTime(const char *path);
    static uint64_t genHash(const void *data, size_t size);

private:
    int m_id_seed = 0;
    std::map<std::string, Record> m_records;
    std::vector<TexturePtr> m_dirty_list;
};

} // namespace ms
