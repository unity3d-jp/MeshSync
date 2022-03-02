#pragma once

#ifndef msRuntime

#include "MeshSync/SceneExporter.h"
#include "MeshSync/SceneCache/msSceneCacheOutput.h"

namespace ms {

struct SceneCacheOutputSettings;

class SceneCacheWriter : public SceneExporter
{
public:
    float time = 0.0f;

public:
    SceneCacheWriter();
    ~SceneCacheWriter() override;

    bool open(const char *path, const SceneCacheOutputSettings& oscs);
    void close();
    bool valid() const;

    bool isExporting() override;
    void wait() override;
    void kick() override;

private:
    static SceneCacheOutputPtr OpenOSceneCacheFile(const char *path, const SceneCacheOutputSettings& oscs);
    static SceneCacheOutput* OpenOSceneCacheFileRaw(const char *path, const SceneCacheOutputSettings& oscs);

    void write();

    SceneCacheOutputPtr m_osc;
    std::string m_error_message;
};

} // namespace ms

#endif // msRuntime
