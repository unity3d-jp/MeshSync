#pragma once

#include "SceneGraph/msMaterial.h"

#ifndef msRuntime
namespace ms {

class TextureManager
{
public:
    TextureManager();
    ~TextureManager();
    bool empty() const;
    void clear();
    bool erase(const std::string& name);
    int find(const std::string& name) const;

    // thread safe
    int addImage(const std::string& name, int width, int height, const void *data, size_t size, TextureFormat format);
    // thread safe
    int addFile(const std::string& path, TextureType type);
    // thread safe
    int add(TexturePtr tex);

    std::vector<TexturePtr> getAllTextures();
    std::vector<TexturePtr> getDirtyTextures();
    void makeDirtyAll();
    void clearDirtyFlags();

    void setAlwaysMarkDirty(bool v);

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
    Record& lockAndGet(const std::string& path);

    int m_id_seed = 0;
    bool m_always_mark_dirty = false;
    std::map<std::string, Record> m_records;
    std::mutex m_mutex;
};

} // namespace ms
#endif // msRuntime
