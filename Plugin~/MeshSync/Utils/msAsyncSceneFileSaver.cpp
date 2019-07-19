#include "pch.h"
#include "msAsyncSceneFileSaver.h"
#include "../MeshSync.h"

namespace ms {

AsyncSceneFileSaver::AsyncSceneFileSaver() : m_scene(nullptr)
{
	m_scene = ms::Scene::create();
}

AsyncSceneFileSaver::~AsyncSceneFileSaver()
{
    wait();
}

void AsyncSceneFileSaver::resetScene() {
	m_scene->assets.clear();
	m_scene->entities.clear();
}


void AsyncSceneFileSaver::AddAsset(AssetPtr asset) {
	m_scene->assets.push_back(asset);
}

void AsyncSceneFileSaver::AddEntity(TransformPtr t) {
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

void AsyncSceneFileSaver::kickManualSave(const std::string& fullPath) {
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

		//In Windows 10, this is C:\Users\sin\AppData\Local\Temp
		std::string fullPath(Poco::Path::cacheHome());
		fullPath+="UTJ\\MeshSync";
		Poco::File f(fullPath);
		f.createDirectories();
		
		//Find time
		time_t rawtime;
		time (&rawtime);
		struct tm * timeinfo = localtime(&rawtime); 
		char date_str[128];

		strftime(date_str,sizeof(date_str),"\\%Y-%m-%d_%H-%M-%S.scz",timeinfo);
		fullPath+=date_str;

		saveToFile(fullPath, false); 
	});
}

void AsyncSceneFileSaver::saveToFile(const std::string& path, const bool forceSave)
{
    if (on_prepare)
        on_prepare(*this);

    if (!forceSave && m_scene->assets.empty() && m_scene->entities.empty())
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

