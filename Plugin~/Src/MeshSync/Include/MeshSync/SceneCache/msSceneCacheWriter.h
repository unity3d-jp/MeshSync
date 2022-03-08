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
    SceneCacheWriter() = default;
    ~SceneCacheWriter() override;

    bool Open(const char *path, const SceneCacheOutputSettings& oscs);
    void Close();
    bool IsValid() const;
    inline void SetTime(float time);

    bool isExporting() override;
    void wait() override;
    void kick() override;

private:
    static SceneCacheOutputFilePtr OpenOSceneCacheFile(const char *path, const SceneCacheOutputSettings& oscs);
    static SceneCacheOutputFile* OpenOSceneCacheFileRaw(const char *path, const SceneCacheOutputSettings& oscs);

    void write();

    SceneCacheOutputFilePtr m_scOutputFile;
    std::string m_errorMessage;
    float m_time = 0.0f;

};

void SceneCacheWriter::SetTime(const float time) { m_time = time; }

} // namespace ms

#endif // msRuntime
