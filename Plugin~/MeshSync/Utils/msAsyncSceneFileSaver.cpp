#include "pch.h"
#include "msAsyncSceneFileSaver.h"
#include "../MeshSync.h"

namespace ms {

AsyncSceneFileSaver::AsyncSceneFileSaver()
    : m_scene(nullptr)
{
    m_scene = ms::Scene::create();
}

AsyncSceneFileSaver::~AsyncSceneFileSaver()
{
    wait();
}

void AsyncSceneFileSaver::resetScene()
{
    m_scene->assets.clear();
    m_scene->entities.clear();
}


void AsyncSceneFileSaver::addAsset(AssetPtr asset)
{
    m_scene->assets.push_back(asset);
}

void AsyncSceneFileSaver::addEntity(std::vector<TransformPtr>& collection)
{
    for (auto& c : collection)
        m_scene->entities.push_back(c);
}


void AsyncSceneFileSaver::addEntity(TransformPtr t)
{
    m_scene->entities.push_back(t);
}


const std::string& AsyncSceneFileSaver::getErrorMessage() const
{
    return m_error_message;
}

bool AsyncSceneFileSaver::isSaving()
{
    if (m_future.valid() && m_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
        return true;

    return false;
}

void AsyncSceneFileSaver::wait()
{
    if (m_future.valid()) {
        m_future.wait();
        m_future = {};
    }
}

void AsyncSceneFileSaver::kickManualSave(const std::string& fullPath)
{
    wait();

    m_future = std::async(std::launch::async, [this, fullPath]() {
        saveToFile(fullPath, false);
    });
}


void AsyncSceneFileSaver::tryKickAutoSave()
{
    if (isSaving())
        return;

    m_future = std::async(std::launch::async, [this]() {
        //In Windows 10, this is C:\Users\[user_name]\AppData\Local\Temp
        std::string full_path(Poco::Path::cacheHome());
        full_path += "UTJ\\MeshSync";
        Poco::File f(full_path);
        f.createDirectories();

        //Limit the number of files
        const uint64_t MAX_CACHE_FILES = 100;
        std::multimap<uint64_t, std::string> existingFiles;
        FindFilesSortedByLastModified(full_path, existingFiles);
        if (existingFiles.size() > MAX_CACHE_FILES) {
            size_t num_files_to_delete = existingFiles.size() - MAX_CACHE_FILES;

            auto enumerator = existingFiles.begin();
            size_t i = 0;
            while (enumerator != existingFiles.end() && i < num_files_to_delete) {

                Poco::File file_to_delete(enumerator->second);
                file_to_delete.remove();

                ++i;
                ++enumerator;
            }
        }

        //Find time
        time_t rawtime;
        time(&rawtime);
        struct tm * timeinfo = localtime(&rawtime);
        char date_str[128];

        strftime(date_str, sizeof(date_str), "\\%Y-%m-%d_%H-%M-%S.scz", timeinfo);
        full_path += date_str;

        saveToFile(full_path, false);
    });
}

void AsyncSceneFileSaver::saveToFile(const std::string& path, const bool force)
{
    if (on_prepare)
        on_prepare(*this);

    if (!force && m_scene->assets.empty() && m_scene->entities.empty())
        return;

    std::sort(m_scene->entities.begin(), m_scene->entities.end(), [](TransformPtr& a, TransformPtr& b) { return a->order < b->order; });

    ms::OSceneCachePtr oscz = ms::OpenOSceneCacheFile(path.c_str(), { ms::SceneCacheEncoding::ZSTD });
    oscz->addScene(m_scene, 0.5f);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    //Wait until the writing is done
    while (oscz->isWriting()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    resetScene();
}

} // namespace ms

