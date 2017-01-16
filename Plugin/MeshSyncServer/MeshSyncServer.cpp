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
    return server->processMessages([handler](ms::MessageData& data) {
        if (auto* get = dynamic_cast<ms::GetData*>(&data)) {
            ms::GetDataCS cs = *get;
            handler(ms::MessageType::Get, &cs);
        }
        else if (auto* del = dynamic_cast<ms::DeleteData*>(&data)) {
            ms::DeleteDataCS cs = *del;
            handler(ms::MessageType::Delete, &cs);
        }
        else if (auto* mesh = dynamic_cast<ms::MeshData*>(&data)) {
            ms::MeshDataCS cs = *mesh;
            handler(ms::MessageType::Mesh, &cs);
        }
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
msAPI void msServerAddServeData(ms::Server *server, ms::MessageType type, const void *data)
{
    if (!server) { return; }
    switch (type) {
    case ms::MessageType::Mesh:
    {
        auto edata = new ms::MeshData(*(const ms::MeshDataCS*)data);
        server->addServeData(edata);
    }
    }
}


static void msCopyData(ms::GetDataCS *dst, const ms::GetDataCS *src)
{
    if (!dst || !src) { return; }

    dst->flags = src->flags;
}
static void msCopyData(ms::DeleteDataCS *dst, const ms::DeleteDataCS *src)
{
    if (!dst || !src) { return; }

    dst->obj_path = src->obj_path;
}
static void msCopyData(ms::MeshDataCS *dst, const ms::MeshDataCS *src)
{
    if (!dst || !src) { return; }

    dst->cpp = src->cpp;
    dst->path = src->path;
    dst->num_points = src->num_points;
    dst->num_indices = src->num_indices;
    dst->num_submeshes = src->num_submeshes;
    dst->transform = src->transform;

    if (src->points) {
        if (dst->points) {
            memcpy(dst->points, src->points, sizeof(float3)*src->num_points);
        }
        else {
            dst->points = src->points;
        }
    }
    if (src->normals) {
        if (dst->normals) {
            memcpy(dst->normals, src->normals, sizeof(float3)*src->num_points);
        }
        else {
            dst->normals = src->normals;
        }
    }
    if (src->tangents) {
        if (dst->tangents) {
            memcpy(dst->tangents, src->tangents, sizeof(float4)*src->num_points);
        }
        else {
            dst->tangents = src->tangents;
        }
    }
    if (src->uv) {
        if (dst->uv) {
            memcpy(dst->uv, src->uv, sizeof(float2)*src->num_points);
        }
        else {
            dst->uv = src->uv;
        }
    }
    if (src->indices) {
        if (dst->indices) {
            memcpy(dst->indices, src->indices, sizeof(int)*src->num_indices);
        }
        else {
            dst->indices = src->indices;
        }
    }
}
msAPI void msCopyData(ms::MessageType et, void *dst, const void *src)
{
    switch (et) {
    case ms::MessageType::Get:
        msCopyData((ms::GetDataCS*)dst, (const ms::GetDataCS*)src);
        break;
    case ms::MessageType::Delete:
        msCopyData((ms::DeleteDataCS*)dst, (const ms::DeleteDataCS*)src);
        break;
    case ms::MessageType::Mesh:
        msCopyData((ms::MeshDataCS*)dst, (const ms::MeshDataCS*)src);
        break;
    }
}

msAPI const char* msCreateString(const char *str)
{
    auto len = strlen(str) + 1;
    auto ret = new char[len];
    memcpy(ret, str, len);
    return ret;
}
msAPI void msDeleteString(const char *str)
{
    delete[] str;
}


msAPI void msGetSubmeshData(ms::SubmeshDataCS *dst, const ms::MeshDataCS *v, int i)
{
    *dst = v->cpp->submeshes[i];
}

msAPI void msCopySubmeshData(ms::SubmeshDataCS *dst, const ms::SubmeshDataCS *src)
{
    dst->num_points = src->num_points;
    dst->num_indices = src->num_indices;

    if (src->points && dst->points) {
        memcpy(dst->points, src->points, sizeof(float3)*src->num_points);
    }
    if (src->normals && dst->normals) {
        memcpy(dst->normals, src->normals, sizeof(float3)*src->num_points);
    }
    if (src->tangents && dst->tangents) {
        memcpy(dst->tangents, src->tangents, sizeof(float4)*src->num_points);
    }
    if (src->uv && dst->uv) {
        memcpy(dst->uv, src->uv, sizeof(float2)*src->num_points);
    }
    if (src->indices && dst->indices) {
        memcpy(dst->indices, src->indices, sizeof(int)*src->num_indices);
    }
}
