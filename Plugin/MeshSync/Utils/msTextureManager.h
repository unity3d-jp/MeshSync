#pragma once

#include "../msMaterial.h"

namespace ms {

class TextureManager
{
public:
    TextureManager();
    ~TextureManager();
    void clear();
    void erase(const std::string& name);
    int find(const std::string& name) const;
    int addImage(const std::string& name, int width, int height, const void *data, size_t size, TextureFormat format);
    int addFile(const std::string& path, TextureType type);

    std::vector<TexturePtr> getAllTextures();
    std::vector<TexturePtr> getDirtyTextures();
    void clearDirtyFlags();

private:
    struct Record
    {
        TexturePtr texture;
        uint64_t mtime = 0;
        uint64_t checksum = 0;
        bool dirty = false;
        std::future<void> task;

        void waitTask();
    };
    int genID();
    void waitTasks();

    int m_id_seed = 0;
    std::map<std::string, Record> m_records;
};

} // namespace ms
