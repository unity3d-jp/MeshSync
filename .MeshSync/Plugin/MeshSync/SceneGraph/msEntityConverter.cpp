#include "pch.h"
#include "msEntity.h"
#include "msMesh.h"
#include "msEntityConverter.h"

namespace ms {

void EntityConverterBase::convert(Entity &e)
{
    switch (e.getType()) {
    case Entity::Type::Transform:
        convertTransform(static_cast<Transform&>(e));
        break;
    case Entity::Type::Camera:
        convertCamera(static_cast<Camera&>(e));
        break;
    case Entity::Type::Light:
        convertLight(static_cast<Light&>(e));
        break;
    case Entity::Type::Mesh:
        convertMesh(static_cast<Mesh&>(e));
        break;
    case Entity::Type::Points:
        convertPoints(static_cast<Points&>(e));
        break;
    default:
        break;
    }
}


void ScaleConverter::convertTransform(Transform &e)
{
    e.position *= scale;
}

void ScaleConverter::convertCamera(Camera &e)
{
    convertTransform(e);
    e.near_plane *= scale;
    e.far_plane *= scale;
}

void ScaleConverter::convertLight(Light &e)
{
    convertTransform(e);
    e.range *= scale;
}

void ScaleConverter::convertMesh(Mesh &e)
{
    convertTransform(e);
    mu::Scale(e.points.data(), scale, e.points.size());
    for (auto& bone : e.bones) {
        (float3&)bone->bindpose[3] *= scale;
    }
    for (auto& bs : e.blendshapes) {
        for (auto& frame : bs->frames) {
            mu::Scale(frame->points.data(), scale, frame->points.size());
        }
    }
}

void ScaleConverter::convertPoints(Points &e)
{
    convertTransform(e);
    for (auto& p : e.data) {
        mu::Scale(p->points.data(), scale, p->points.size());
    }
}



void FlipXConverter::convertTransform(Transform &e)
{
    e.position = flip_x(e.position);
    e.rotation = flip_x(e.rotation);
}

void FlipXConverter::convertCamera(Camera &e)
{
    convertTransform(e);
}

void FlipXConverter::convertLight(Light &e)
{
    convertTransform(e);
}

void FlipXConverter::convertMesh(Mesh &e)
{
    convertTransform(e);

    mu::InvertX(e.points.data(), e.points.size());
    mu::InvertX(e.normals.data(), e.normals.size());
    mu::InvertX(e.tangents.data(), e.tangents.size());
    mu::InvertX(e.velocities.data(), e.velocities.size());

    for (auto& bone : e.bones) {
        bone->bindpose = flip_x(bone->bindpose);
    }
    for (auto& bs : e.blendshapes) {
        for (auto& frame : bs->frames) {
            for (auto& v : frame->points) { v = flip_x(v); }
            for (auto& v : frame->normals) { v = flip_x(v); }
            for (auto& v : frame->tangents) { v = flip_x(v); }
        }
    }
}

void FlipXConverter::convertPoints(Points &e)
{
    convertTransform(e);
    for (auto& p : e.data) {
        mu::InvertX(p->points.data(), p->points.size());
        for (auto& v : p->rotations)
            v = flip_x(v);
        mu::InvertX(p->scales.data(), p->scales.size());
    }
}



void FlipYZConverter::convertTransform(Transform &e)
{
    e.position = swap_yz(e.position);
    e.rotation = swap_yz(e.rotation);
    e.scale = swap_yz(e.scale);
}

void FlipYZConverter::convertCamera(Camera &e)
{
    convertTransform(e);
}

void FlipYZConverter::convertLight(Light &e)
{
    convertTransform(e);
}

void FlipYZConverter::convertMesh(Mesh &e)
{
    convertTransform(e);

    for (auto& v : e.points) v = swap_yz(v);
    for (auto& v : e.normals) v = swap_yz(v);
    for (auto& v : e.tangents) v = swap_yz(v);
    for (auto& v : e.velocities) v = swap_yz(v);

    for (auto& bone : e.bones) {
        bone->bindpose = swap_yz(bone->bindpose);
    }
    for (auto& bs : e.blendshapes) {
        for (auto& frame : bs->frames) {
            for (auto& v : frame->points) { v = swap_yz(v); }
            for (auto& v : frame->normals) { v = swap_yz(v); }
            for (auto& v : frame->tangents) { v = swap_yz(v); }
        }
    }
}

void FlipYZConverter::convertPoints(Points &e)
{
    convertTransform(e);
    for (auto& p : e.data) {
        for (auto& v : p->points) v = swap_yz(v);
        for (auto& v : p->rotations) v = swap_yz(v);
        for (auto& v : p->scales) v = swap_yz(v);
    }
}



void FBXZUpToYUpConvertex::convertTransform(Transform &v)
{
    if (v.isRoot()) {
        v.position = flip_z(swap_yz(v.position));
        v.rotation = flip_z(swap_yz(v.rotation)) * rotate_x(-90.0f * DegToRad);
        v.scale = swap_yz(v.scale);
    }
}

void FBXZUpToYUpConvertex::convertCamera(Camera &v)
{
    convertTransform(v);
}

void FBXZUpToYUpConvertex::convertLight(Light &v)
{
    convertTransform(v);
}

void FBXZUpToYUpConvertex::convertMesh(Mesh &v)
{
    convertTransform(v);
}

void FBXZUpToYUpConvertex::convertPoints(Points &v)
{
    convertTransform(v);
}

} // namespace ms
