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
        handler(data.type, &data);
    });
}

msAPI void  msServerStop(ms::Server *server)
{
    delete server;
}

msAPI void  msCopyEditData(ms::EditDataRef *dst, const ms::EditDataRef *src)
{
    if (!dst || !src) { return; }

    dst->num_points = src->num_points;
    dst->num_indices = src->num_indices;
    dst->position = src->position;

    if (src->points) {
        if (dst->points) {
            memcpy(dst->points, src->points, sizeof(ms::float3)*src->num_points);
        }
        else {
            dst->points = src->points;
        }
    }
    if (src->normals) {
        if (dst->normals) {
            memcpy(dst->normals, src->normals, sizeof(ms::float3)*src->num_points);
        }
        else {
            dst->normals = src->normals;
        }
    }
    if (src->tangents) {
        if (dst->tangents) {
            memcpy(dst->tangents, src->tangents, sizeof(ms::float4)*src->num_points);
        }
        else {
            dst->tangents = src->tangents;
        }
    }
    if (src->uv) {
        if (dst->uv) {
            memcpy(dst->uv, src->uv, sizeof(ms::float2)*src->num_points);
        }
        else {
            dst->uv = src->uv;
        }
    }
}
