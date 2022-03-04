#pragma once

#ifndef msRuntime

#include "MeshSync/SceneExporter.h"

msDeclClassPtr(SceneCacheOutputFile)

namespace ms {

struct SceneCacheOutputSettings;
class SceneCacheOutputFile;

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
    static SceneCacheOutputFilePtr OpenOSceneCacheFile(const char *path, const SceneCacheOutputSettings& oscs);
    static SceneCacheOutputFile* OpenOSceneCacheFileRaw(const char *path, const SceneCacheOutputSettings& oscs);

    void write();

    SceneCacheOutputFilePtr m_osc;
    std::string m_error_message;
};

} // namespace ms

#endif // msRuntime
