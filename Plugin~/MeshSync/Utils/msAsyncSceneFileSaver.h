#pragma once

#include "../SceneGraph/msSceneGraph.h"

namespace ms {

class AsyncSceneFileSaver
{
public:

    SceneSettings scene_settings;

    std::function<void(AsyncSceneFileSaver&)> on_prepare;
    std::function<void()> on_success, on_error, on_complete;


    AsyncSceneFileSaver();
    ~AsyncSceneFileSaver();

    //[TODO-sindharta: 2019-7-22] All these should use const, like "shared_ptr<const Asset>"
    void AddAsset(AssetPtr);
    template <typename T>
    void AddAsset(std::vector<T>& col) {
        auto& enumerator = col.begin();
        while (enumerator != col.end()) {
    	    m_scene->assets.push_back(*enumerator);
            ++enumerator;
        }
    }
    void AddEntity(TransformPtr);
    void AddEntity(std::vector<TransformPtr>&);

    void resetScene();
    const std::string& getErrorMessage() const;
    bool isSaving();
    void wait();
    void tryKickAutoSave();
    void kickManualSave(const std::string& path);

private:
    void saveToFile(const std::string& path, const bool force);

    std::shared_ptr<Scene> m_scene = nullptr;
    std::future<void> m_future;
    std::string m_error_message;
};

} // namespace ms
