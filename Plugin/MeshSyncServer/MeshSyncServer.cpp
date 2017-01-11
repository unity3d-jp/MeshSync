#include "pch.h"
#include "MeshSync/msServer.h"
#include "MeshSyncServer.h"


msAPI ms::Server* msServerStart(const ms::ServerSettings *settings)
{
    ms::Server *ret = nullptr;
    if (settings) {
        ret = new ms::Server(*settings);
        if (!ret->start()) {
            delete ret;
            ret = nullptr;
        }
    }
    return ret;
}

msAPI void msServerProcessEvents(ms::Server *server, msEventHandler handler)
{
    if (!server) { return; }
    server->processEvents([handler](ms::EventData& data) {
        if (data.type == ms::EventType::Delete) {
            ms::DeleteDataCS ref = (ms::DeleteData&)data;
            handler(data.type, &ref);
        }
        else if (data.type == ms::EventType::Xform) {
            ms::XformDataCS ref = (ms::XformData&)data;
            handler(data.type, &ref);
        }
        else if (data.type == ms::EventType::Mesh) {
            ms::MeshDataCS ref = (ms::MeshData&)data;
            handler(data.type, &ref);
        }
    });
}

msAPI void  msServerStop(ms::Server *server)
{
    delete server;
}


static void msCopyData(ms::DeleteDataCS *dst, const ms::DeleteDataCS *src)
{
    if (!dst || !src) { return; }

    dst->obj_path = src->obj_path;
}
static void msCopyData(ms::XformDataCS *dst, const ms::XformDataCS *src)
{
    if (!dst || !src) { return; }

    dst->obj_path  = src->obj_path;
    dst->position  = src->position;
    dst->rotation  = src->rotation;
    dst->scale     = src->scale;
    dst->transform = src->transform;
}
static void msCopyData(ms::MeshDataCS *dst, const ms::MeshDataCS *src)
{
    if (!dst || !src) { return; }

    dst->obj_path = src->obj_path;
    dst->num_points = src->num_points;
    dst->num_indices = src->num_indices;

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
msAPI void msCopyData(ms::EventType et, void *dst, const void *src)
{
    switch (et) {
    case ms::EventType::Delete:
        msCopyData((ms::DeleteDataCS*)dst, (const ms::DeleteDataCS*)src);
        break;
    case ms::EventType::Xform:
        msCopyData((ms::XformDataCS*)dst, (const ms::XformDataCS*)src);
        break;
    case ms::EventType::Mesh:
        msCopyData((ms::MeshDataCS*)dst, (const ms::MeshDataCS*)src);
        break;
    }
}
