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
    return server->processMessages([handler](ms::MessageType type, const ms::Message& data) {
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
msAPI void msServerServeMesh(ms::Server *server, ms::Mesh *data)
{
    if (!server) { return; }
    server->getHostScene()->meshes.emplace_back(data);
}
msAPI void msServerSetNumMaterials(ms::Server *server, int n)
{
    if (!server) { return; }
    server->getHostScene()->materials.resize(n);
}


msAPI void msServerSetScreenshotFilePath(ms::Server *server, const char *path)
{
    if (!server) { return; }
    server->setScrrenshotFilePath(path);
}

msAPI int msGetGetBakeSkin(ms::GetMessage *_this)
{
    return _this->refine_settings.flags.bake_skin;
}
msAPI int msGetGetBakeCloth(ms::GetMessage *_this)
{
    return _this->refine_settings.flags.bake_cloth;
}

msAPI int msSetGetNumMeshes(ms::SetMessage *_this)
{
    return (int)_this->scene.meshes.size();
}
msAPI ms::Mesh* msSetGetMeshData(ms::SetMessage *_this, int i)
{
    return _this->scene.meshes[i].get();
}
msAPI int msSetGetNumTransforms(ms::SetMessage *_this)
{
    return (int)_this->scene.transforms.size();
}
msAPI ms::Transform* msSetGetTransformData(ms::SetMessage *_this, int i)
{
    return _this->scene.transforms[i].get();
}
msAPI int msSetGetNumCameras(ms::SetMessage *_this)
{
    return (int)_this->scene.cameras.size();
}
msAPI ms::Camera* msSetGetCameraData(ms::SetMessage *_this, int i)
{
    return _this->scene.cameras[i].get();
}
msAPI int msSetGetNumMaterials(ms::SetMessage *_this)
{
    return (int)_this->scene.materials.size();
}
msAPI ms::Material* msSetGetMaterialData(ms::SetMessage *_this, int i)
{
    return &_this->scene.materials[i];
}

msAPI int msMaterialGetID(ms::Material *_this)
{
    return _this->id;
}
msAPI void msMaterialSetID(ms::Material *_this, int v)
{
    _this->id = v;
}
msAPI const char* msMaterialGetName(ms::Material *_this)
{
    return _this->name.c_str();
}
msAPI void msMaterialSetName(ms::Material *_this, const char *v)
{
    _this->name = v;
}
msAPI float4 msMaterialGetColor(ms::Material *_this)
{
    return _this->color;
}
msAPI void msMaterialSetColor(ms::Material *_this, const float4 *v)
{
    _this->color = *v;
}


msAPI ms::GetFlags msGetGetFlags(ms::GetMessage *_this)
{
    return _this->flags;
}

msAPI int msDeleteGetNumTargets(ms::DeleteMessage *_this)
{
    return (int)_this->targets.size();
}
msAPI const char* msDeleteGetPath(ms::DeleteMessage *_this, int i)
{
    return _this->targets[i].path.c_str();
}
msAPI int msDeleteGetID(ms::DeleteMessage *_this, int i)
{
    return _this->targets[i].id;
}


msAPI ms::Mesh* msMeshCreate()
{
    return new ms::Mesh();
}
msAPI int msMeshGetID(ms::Mesh *_this)
{
    return _this->id;
}
msAPI void msMeshSetID(ms::Mesh *_this, int v)
{
    _this->id = v;
}
msAPI int msMeshGetIndex(ms::Mesh *_this)
{
    return _this->index;
}
msAPI void msMeshSetIndex(ms::Mesh *_this, int v)
{
    _this->index = v;
}
msAPI ms::MeshDataFlags msMeshGetFlags(ms::Mesh *_this)
{
    return _this->flags;
}
msAPI void msMeshSetFlags(ms::Mesh *_this, ms::MeshDataFlags v)
{
    _this->flags = v;
}
msAPI const char* msMeshGetPath(ms::Mesh *_this)
{
    return _this->path.c_str();
}
msAPI void msMeshSetPath(ms::Mesh *_this, const char *v)
{
    _this->path = v;
}
msAPI int msMeshGetNumPoints(ms::Mesh *_this)
{
    return (int)_this->points.size();
}
msAPI int msMeshGetNumIndices(ms::Mesh *_this)
{
    return (int)_this->indices.size();
}
msAPI int msMeshGetNumSplits(ms::Mesh *_this)
{
    return (int)_this->splits.size();
}
msAPI void msMeshReadPoints(ms::Mesh *_this, float3 *dst)
{
    memcpy(dst, _this->points.data(), sizeof(float3) * _this->points.size());
}
msAPI void msMeshWritePoints(ms::Mesh *_this, const float3 *v, int size)
{
    _this->points.assign(v, v + size);
    _this->flags.has_points = 1;
}
msAPI void msMeshReadNormals(ms::Mesh *_this, float3 *dst)
{
    memcpy(dst, _this->normals.data(), sizeof(float3) * _this->normals.size());
}
msAPI void msMeshWriteNormals(ms::Mesh *_this, const float3 *v, int size)
{
    _this->normals.assign(v, v + size);
    _this->flags.has_normals = 1;
}
msAPI void msMeshReadTangents(ms::Mesh *_this, float4 *dst)
{
    memcpy(dst, _this->tangents.data(), sizeof(float4) * _this->tangents.size());
}
msAPI void msMeshWriteTangents(ms::Mesh *_this, const float4 *v, int size)
{
    _this->tangents.assign(v, v + size);
    _this->flags.has_tangents = 1;
}
msAPI void msMeshReadUV(ms::Mesh *_this, float2 *dst)
{
    memcpy(dst, _this->uv.data(), sizeof(float2) * _this->uv.size());
}
msAPI void msMeshWriteUV(ms::Mesh *_this, const float2 *v, int size)
{
    _this->uv.assign(v, v + size);
    _this->flags.has_uv = 1;
}
msAPI void msMeshReadIndices(ms::Mesh *_this, int *dst)
{
    memcpy(dst, _this->indices.data(), sizeof(int) * _this->indices.size());
}
msAPI void msMeshWriteIndices(ms::Mesh *_this, const int *v, int size)
{
    _this->indices.assign(v, v + size);
    _this->flags.has_indices = 1;
    _this->flags.visible = 1;
}
msAPI void msMeshWriteSubmeshTriangles(ms::Mesh *_this, const int *v, int size, int materialID)
{
    {
        _this->indices.insert(_this->indices.end(), v, v + size);
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
msAPI ms::SplitData* msMeshGetSplit(ms::Mesh *_this, int i)
{
    return &_this->splits[i];
}
msAPI void msMeshGetTransform(ms::Mesh *_this, ms::TRS *dst)
{
    *dst = _this->transform;
}
msAPI void msMeshSetTransform(ms::Mesh *_this, const ms::TRS *v)
{
    _this->transform = *v;
}
msAPI int msMeshGetNumSubmeshes(ms::Mesh *_this)
{
    return (int)_this->submeshes.size();
}
msAPI ms::SubmeshData* msMeshGetSubmesh(ms::Mesh *_this, int i)
{
    return &_this->submeshes[i];
}

msAPI void msMeshWriteWeights4(ms::Mesh *_this, const ms::Weights4 *v, int size)
{
    _this->bones_par_vertex = 4;
    _this->bone_weights.resize(size * 4);
    _this->bone_indices.resize(size * 4);
    for (int i = 0; i < size; ++i) {
        memcpy(&_this->bone_weights[i * 4], v[i].weight, sizeof(float) * 4);
        memcpy(&_this->bone_indices[i * 4], v[i].indices, sizeof(int) * 4);
    }
}
msAPI void msMeshSetBone(ms::Mesh *_this, const char *v, int i)
{
    if (i + 1 >= _this->bones.size()) {
        _this->bones.resize(i + 1);
    }
    _this->bones[i] = v;
}
msAPI void msMeshWriteBindPoses(ms::Mesh *_this, const float4x4 *v, int size)
{
    _this->bindposes.assign(v, v + size);
}
msAPI void msMeshSetLocal2World(ms::Mesh *_this, const float4x4 *v)
{
    _this->refine_settings.local2world = *v;
}
msAPI void msMeshSetWorld2Local(ms::Mesh *_this, const float4x4 *v)
{
    _this->refine_settings.world2local = *v;
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
