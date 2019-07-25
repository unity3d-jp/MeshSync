#pragma once

#include "../SceneCache/msSceneCache.h"

namespace ms {

class AsyncSceneFileSaver
{
public:
    SceneSettings scene_settings;
    std::vector<AssetPtr> assets;
    std::vector<TexturePtr> textures;
    std::vector<MaterialPtr> materials;
    std::vector<TransformPtr> transforms;
    std::vector<TransformPtr> geometries;
    std::vector<AnimationClipPtr> animations;

    std::function<void()> on_prepare, on_success, on_error, on_complete;


    AsyncSceneFileSaver();
    ~AsyncSceneFileSaver();
    void clear();

    bool open(const char *path, const OSceneCacheSettings& settings = OSceneCacheSettings());
    void close();
    bool valid() const;

    bool isSaving();
    void wait();
    void write(float time);

private:
    OSceneCachePtr m_osc;
    std::string m_error_message;
};

} // namespace ms
