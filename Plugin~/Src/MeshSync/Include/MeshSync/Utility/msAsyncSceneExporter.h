#pragma once

#include "msIDGenerator.h"

#include "MeshSync/MeshSync.h"
#include "MeshSync/msClient.h"
#include "MeshSync/SceneCache/msSceneCacheSettings.h"
#include "MeshSync/SceneGraph/msSceneSettings.h"

#ifndef msRuntime

//Forward declarations
msDeclClassPtr(Asset)
msDeclClassPtr(AnimationClip)
msDeclClassPtr(Scene)
msDeclClassPtr(Texture)
msDeclClassPtr(Transform)
msDeclClassPtr(Material)

namespace ms {

class AsyncSceneExporter
{
public:
    SceneSettings scene_settings;
    std::vector<AssetPtr> assets;
    std::vector<TexturePtr> textures;
    std::vector<MaterialPtr> materials;
    std::vector<TransformPtr> transforms;
    std::vector<TransformPtr> geometries;
    std::vector<AnimationClipPtr> animations;

    std::vector<Identifier> deleted_entities;
    std::vector<Identifier> deleted_materials;

    std::function<void()> on_prepare, on_success, on_error, on_complete;
    PathToID id_table;

public:
    virtual ~AsyncSceneExporter();
    virtual void clear();

    virtual bool isExporting() = 0;
    virtual void wait() = 0;
    virtual void kick() = 0;

    void add(ScenePtr scene);
};


class AsyncSceneSender : public AsyncSceneExporter
{
public:
    int session_id = InvalidID;
    int message_count = 0;

    ClientSettings client_settings;

public:
    AsyncSceneSender(int session_id = InvalidID);
    ~AsyncSceneSender() override;

    const std::string& getErrorMessage() const;
    bool isServerAvaileble();

    bool isExporting() override;
    void wait() override;
    void kick() override;

private:
    void send();

    std::future<void> m_future;
    std::string m_error_message;
};
#endif // msRuntime


#ifdef msEnableSceneCache
class AsyncSceneCacheWriter : public AsyncSceneExporter
{
public:
    float time = 0.0f;

public:
    AsyncSceneCacheWriter();
    ~AsyncSceneCacheWriter() override;

    bool open(const char *path, const OSceneCacheSettings& oscs = OSceneCacheSettings());
    void close();
    bool valid() const;

    bool isExporting() override;
    void wait() override;
    void kick() override;

private:
    void write();

    OSceneCachePtr m_osc;
    std::string m_error_message;
};
#endif // msEnableSceneCache

} // namespace ms
