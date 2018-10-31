#pragma once

#include "../msClient.h"

namespace ms {

class AsyncSceneSender
{
public:
    ClientSettings client_settings;
    SceneSettings scene_settings;
    std::vector<TexturePtr> textures;
    std::vector<MaterialPtr> materials;
    std::vector<TransformPtr> transforms;
    std::vector<TransformPtr> geometries;
    std::vector<AnimationClipPtr> animations;
    std::vector<Identifier> deleted;

    std::function<void()> on_prepare, on_succeeded, on_failed;

    AsyncSceneSender();
    ~AsyncSceneSender();
    bool isSending();
    void wait();
    void kick();

private:
    void send();

    std::future<void> m_future;
};

} // namespace ms
