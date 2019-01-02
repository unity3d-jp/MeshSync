#pragma once

#include "../msClient.h"

namespace ms {

class AsyncSceneSender
{
public:
    int session_id = InvalidID;
    int message_count = 0;

    ClientSettings client_settings;
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

    AsyncSceneSender(int session_id = InvalidID);
    ~AsyncSceneSender();
    bool isSending();
    void wait();
    void kick();

private:
    void send();

    std::future<void> m_future;
};

} // namespace ms
