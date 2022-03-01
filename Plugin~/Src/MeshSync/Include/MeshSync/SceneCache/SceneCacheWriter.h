#pragma once

#ifndef msRuntime

#include "MeshSync/SceneExporter.h"
#include "MeshSync/SceneCache/msSceneCacheOutputSettings.h"

namespace ms {


class SceneCacheWriter : public SceneExporter
{
public:
    float time = 0.0f;

public:
    SceneCacheWriter();
    ~SceneCacheWriter() override;

    bool open(const char *path, const OSceneCacheSettings& oscs = OSceneCacheSettings());
    void close();
    bool valid() const;

    bool isExporting() override;
    void wait() override;
    void kick() override;

private:
    void write();

    SceneCacheOutputPtr m_osc;
    std::string m_error_message;
};

} // namespace ms

#endif // msRuntime
