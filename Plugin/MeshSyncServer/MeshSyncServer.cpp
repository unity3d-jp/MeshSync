#include "pch.h"
#include "MeshSync/msServer.h"
#include "MeshSyncServer.h"

static std::map<uint16_t, ms::Server*> g_servers;

msAPI ms::Server* msServerStart(const ms::ServerSettings *settings)
{
    if (!settings) { return nullptr; }

    ms::Server *ret = g_servers[settings->port];
    if (!ret) {
        ret = new ms::Server(*settings);
        ret->start();
        g_servers[settings->port] = ret;
    }
    else {
        ret->setServe(true);
        ret->getSettings() = *settings;
    }
    return ret;
}

msAPI void  msServerStop(ms::Server *server)
{
    // actually not stop. just make server ignore further requests.
    if (server) {
        server->setServe(false);
    }
}

msAPI int msServerProcessMessages(ms::Server *server, msMessageHandler handler)
{
    if (!server || !handler) { return 0; }
    return server->processMessages([handler](ms::MessageType type, const ms::MessageData& data) {
        handler(type, &data);
    });
}

msAPI void msServerBeginServe(ms::Server *server)
{
    if (!server) { return; }
    server->beginServe();
}
msAPI void msServerEndServe(ms::Server *server)
{
    if (!server) { return; }
    server->endServe();
}
msAPI void msServerAddServeData(ms::Server *server, ms::MessageType type, void *data)
{
    if (!server) { return; }
    switch (type) {
    case ms::MessageType::Mesh:
    {
        server->addServeData((ms::MeshData*)data);
    }
    }
}


msAPI void msServerSetScreenshotFilePath(ms::Server *server, const char *path)
{
    if (!server) { return; }
    server->setScrrenshotFilePath(path);
}


msAPI ms::GetFlags msGetGetFlags(ms::GetData *_this)
{
    return _this->flags;
}

msAPI const char* msDeleteGetPath(ms::DeleteData *_this)
{
    return _this->path.c_str();
}
msAPI int msDeleteGetID(ms::DeleteData *_this)
{
    return _this->id;
}


msAPI ms::MeshData* msMeshCreate()
{
    return new ms::MeshData();
}
msAPI int msMeshGetID(ms::MeshData *_this)
{
    return _this->id;
}
msAPI void msMeshSetID(ms::MeshData *_this, int v)
{
    _this->id = v;
}
msAPI ms::MeshDataFlags msMeshGetFlags(ms::MeshData *_this)
{
    return _this->flags;
}
msAPI void msMeshSetFlags(ms::MeshData *_this, ms::MeshDataFlags v)
{
    _this->flags = v;
}
msAPI const char* msMeshGetPath(ms::MeshData *_this)
{
    return _this->path.c_str();
}
msAPI void msMeshSetPath(ms::MeshData *_this, const char *v)
{
    _this->path = v;
}
msAPI int msMeshGetNumPoints(ms::MeshData *_this)
{
    return (int)_this->points.size();
}
msAPI int msMeshGetNumIndices(ms::MeshData *_this)
{
    return (int)_this->indices.size();
}
msAPI int msMeshGetNumSplits(ms::MeshData *_this)
{
    return (int)_this->splits.size();
}
msAPI void msMeshReadPoints(ms::MeshData *_this, float3 *dst)
{
    memcpy(dst, _this->points.data(), sizeof(float3) * _this->points.size());
}
msAPI void msMeshWritePoints(ms::MeshData *_this, const float3 *v, int size)
{
    _this->points.resize(size);
    memcpy(_this->points.data(), v, sizeof(float3) * size);
    _this->flags.has_points = 1;
}
msAPI void msMeshReadNormals(ms::MeshData *_this, float3 *dst)
{
    memcpy(dst, _this->normals.data(), sizeof(float3) * _this->normals.size());
}
msAPI void msMeshWriteNormals(ms::MeshData *_this, const float3 *v, int size)
{
    _this->normals.resize(size);
    memcpy(_this->normals.data(), v, sizeof(float3) * size);
    _this->flags.has_normals = 1;
}
msAPI void msMeshReadTangents(ms::MeshData *_this, float4 *dst)
{
    memcpy(dst, _this->tangents.data(), sizeof(float4) * _this->tangents.size());
}
msAPI void msMeshWriteTangents(ms::MeshData *_this, const float4 *v, int size)
{
    _this->tangents.resize(size);
    memcpy(_this->tangents.data(), v, sizeof(float4) * size);
    _this->flags.has_tangents = 1;
}
msAPI void msMeshReadUV(ms::MeshData *_this, float2 *dst)
{
    memcpy(dst, _this->uv.data(), sizeof(float2) * _this->uv.size());
}
msAPI void msMeshWriteUV(ms::MeshData *_this, const float2 *v, int size)
{
    _this->uv.resize(size);
    memcpy(_this->uv.data(), v, sizeof(float2) * size);
    _this->flags.has_uv = 1;
}
msAPI void msMeshReadIndices(ms::MeshData *_this, int *dst)
{
    memcpy(dst, _this->indices.data(), sizeof(int) * _this->indices.size());
}
msAPI void msMeshWriteIndices(ms::MeshData *_this, const int *v, int size)
{
    _this->indices.resize(size);
    memcpy(_this->indices.data(), v, sizeof(int) * size);
    _this->flags.has_indices = 1;
    _this->flags.visible = 1;
}
msAPI void msMeshWriteSubmeshTriangles(ms::MeshData *_this, const int *v, int size, int materialID)
{
    {
        size_t pos = _this->indices.size();
        _this->indices.resize(pos + size);
        memcpy(_this->indices.data() + pos, v, sizeof(int) * size);
    }
    {
        size_t pos = _this->materialIDs.size();
        _this->materialIDs.resize(pos + size / 3);
        std::fill_n(_this->materialIDs.data() + pos, size / 3, materialID);
    }
    _this->flags.has_indices = 1;
    _this->flags.has_materialIDs = 1;
    _this->flags.visible = 1;
}
msAPI ms::SplitData* msMeshGetSplit(ms::MeshData *_this, int i)
{
    return &_this->splits[i];
}
msAPI void msMeshGetTransform(ms::MeshData *_this, ms::Transform *dst)
{
    *dst = _this->transform;
}
msAPI void msMeshSetTransform(ms::MeshData *_this, const ms::Transform *v)
{
    _this->transform = *v;
    _this->flags.has_transform = 1;
}
msAPI int msMeshGetNumSubmeshes(ms::MeshData *_this)
{
    return (int)_this->submeshes.size();
}
msAPI ms::SubmeshData* msMeshGetSubmesh(ms::MeshData *_this, int i)
{
    return &_this->submeshes[i];
}


msAPI int msSplitGetNumPoints(ms::SplitData *_this)
{
    return (int)_this->points.size();
}
msAPI int msSplitGetNumIndices(ms::SplitData *_this)
{
    return (int)_this->indices.size();
}
msAPI int msSplitGetNumSubmeshes(ms::SplitData *_this)
{
    return (int)_this->submeshes.size();
}
msAPI int msSplitReadPoints(ms::SplitData *_this, float3 *dst)
{
    memcpy(dst, _this->points.data(), sizeof(float3) * _this->points.size());
    return (int)_this->points.size();
}
msAPI int msSplitReadNormals(ms::SplitData *_this, float3 *dst)
{
    memcpy(dst, _this->normals.data(), sizeof(float3) * _this->normals.size());
    return (int)_this->normals.size();
}
msAPI int msSplitReadTangents(ms::SplitData *_this, float4 *dst)
{
    memcpy(dst, _this->tangents.data(), sizeof(float4) * _this->tangents.size());
    return (int)_this->tangents.size();
}
msAPI int msSplitReadUV(ms::SplitData *_this, float2 *dst)
{
    memcpy(dst, _this->uv.data(), sizeof(float2) * _this->uv.size());
    return (int)_this->uv.size();
}
msAPI int msSplitReadIndices(ms::SplitData *_this, int *dst)
{
    memcpy(dst, _this->indices.data(), sizeof(int) * _this->indices.size());
    return (int)_this->indices.size();
}
msAPI ms::SubmeshData* msSplitGetSubmesh(ms::SplitData *_this, int i)
{
    return &_this->submeshes[i];
}


msAPI int msSubmeshGetNumIndices(ms::SubmeshData *_this)
{
    return (int)_this->indices.size();
}
msAPI int msSubmeshGetMaterialID(ms::SubmeshData *_this)
{
    return _this->materialID;
}
msAPI int msSubmeshReadIndices(ms::SubmeshData *_this, int *dst)
{
    memcpy(dst, _this->indices.data(), sizeof(int) * _this->indices.size());
    return (int)_this->indices.size();
}
