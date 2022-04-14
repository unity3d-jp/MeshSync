#include "pch.h"
#include "MeshSync/SceneGraph/msEntityConverter.h"

#include "MeshSync/SceneGraph/msAnimation.h"
#include "MeshSync/SceneGraph/msCamera.h"
#include "MeshSync/SceneGraph/msLight.h"
#include "MeshSync/SceneGraph/msMesh.h"
#include "MeshSync/SceneGraph/msPoints.h"
#include "MeshSync/SceneGraph/msCurve.h"

namespace ms {

void EntityConverter::convert(Entity &e)
{
    switch (e.getType()) {
    case EntityType::Transform:
        convertTransform(dynamic_cast<Transform&>(e));
        break;
    case EntityType::Camera:
        convertCamera(dynamic_cast<Camera&>(e));
        break;
    case EntityType::Light:
        convertLight(dynamic_cast<Light&>(e));
        break;
    case EntityType::Mesh:
        convertMesh(dynamic_cast<Mesh&>(e));
        break;
    case EntityType::Points:
        convertPoints(dynamic_cast<Points&>(e));
        break;
    case EntityType::Curve:
        convertCurve(dynamic_cast<Curve&>(e));
        break;
    default:
        break;
    }
}

void EntityConverter::convert(AnimationClip& clip)
{
    for (auto& anim : clip.animations)
        convert(*anim);
}

void EntityConverter::convert(Animation &anim)
{
    for (auto& curve : anim.curves)
        convertAnimationCurve(*curve);
}

void EntityConverter::convertAnimationCurve(AnimationCurve &/*v*/)
{
}



std::shared_ptr<ScaleConverter> ScaleConverter::create(float scale)
{
    return std::make_shared<ScaleConverter>(scale);
}

ScaleConverter::ScaleConverter(float scale)
    : m_scale(scale)
{
}

void ScaleConverter::convertTransform(Transform &e)
{
    e.position *= m_scale;
}

void ScaleConverter::convertCamera(Camera &e)
{
    convertTransform(e);
    e.near_plane *= m_scale;
    e.far_plane *= m_scale;
    (mu::float3&)e.view_matrix[3] *= m_scale;
    (mu::float3&)e.proj_matrix[3] *= m_scale;
}

void ScaleConverter::convertLight(Light &e)
{
    convertTransform(e);
    e.range *= m_scale;
}

void ScaleConverter::convertMesh(Mesh &e)
{
    convertTransform(e);
    mu::Scale(e.points.data(), m_scale, e.points.size());
    for (auto& bone : e.bones) {
        (mu::float3&)bone->bindpose[3] *= m_scale;
    }
    for (auto& bs : e.blendshapes) {
        for (auto& frame : bs->frames) {
            mu::Scale(frame->points.data(), m_scale, frame->points.size());
        }
    }
}

void ScaleConverter::convertCurve(Curve& e){
    convertTransform(e);
 
    for (auto& spline : e.splines) {
        mu::Scale(spline->cos.data(), m_scale, spline->cos.size());
        mu::Scale(spline->handles_left.data(), m_scale, spline->handles_left.size());
        mu::Scale(spline->handles_right.data(), m_scale, spline->handles_right.size());
    } 
}

void ScaleConverter::convertPoints(Points &e)
{
    convertTransform(e);
    mu::Scale(e.points.data(), m_scale, e.points.size());
}

void ScaleConverter::convertAnimationCurve(AnimationCurve &c)
{
    if (!c.data_flags.affect_scale)
        return;

    switch (c.data_type) {
    case Animation::DataType::Float:
        c.each<float>([this](auto& v) { v.value *= m_scale; });
        break;
    case Animation::DataType::Float2:
        c.each<mu::float2>([this](auto& v) { v.value *= m_scale; });
        break;
    case Animation::DataType::Float3:
        c.each<mu::float3>([this](auto& v) { v.value *= m_scale; });
        break;
    case Animation::DataType::Float4:
        c.each<mu::float4>([this](auto& v) { v.value *= m_scale; });
        break;
    default:
        break;
    }
}


std::shared_ptr<FlipX_HandednessCorrector> FlipX_HandednessCorrector::create()
{
    return std::make_shared<FlipX_HandednessCorrector>();
}

void FlipX_HandednessCorrector::convertTransform(Transform &e)
{
    e.position = flip_x(e.position);
    e.rotation = flip_x(e.rotation);
}

void FlipX_HandednessCorrector::convertCamera(Camera &e)
{
    convertTransform(e);
    e.view_matrix = flip_x(e.view_matrix);
    e.proj_matrix = flip_x(e.proj_matrix);
}

void FlipX_HandednessCorrector::convertLight(Light &e)
{
    convertTransform(e);
}

void FlipX_HandednessCorrector::convertMesh(Mesh &e)
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

void FlipX_HandednessCorrector::convertCurve(Curve& e) {
    convertTransform(e);

    for (auto& spline : e.splines) {
        mu::InvertX(spline->cos.data(), spline->cos.size());
        mu::InvertX(spline->handles_left.data(), spline->handles_left.size());
        mu::InvertX(spline->handles_right.data(), spline->handles_right.size());
    }
}

void FlipX_HandednessCorrector::convertPoints(Points &e)
{
    convertTransform(e);

    mu::InvertX(e.points.data(), e.points.size());
    mu::InvertX(e.scales.data(), e.scales.size());
    for (auto& v : e.rotations)
        v = flip_x(v);
}

void FlipX_HandednessCorrector::convertAnimationCurve(AnimationCurve &c)
{
    if (!c.data_flags.affect_handedness || c.data_flags.ignore_negate)
        return;

    switch (c.data_type) {
    case Animation::DataType::Float3:
        c.each<mu::float3>([this](auto& v) { v.value = flip_x(v.value); });
        break;
    case Animation::DataType::Float4:
        c.each<mu::float4>([this](auto& v) { v.value = flip_x(v.value); });
        break;
    case Animation::DataType::Quaternion:
        c.each<mu::quatf>([this](auto& v) { v.value = flip_x(v.value); });
        break;
    default:
        break;
    }
}



std::shared_ptr<FlipYZ_ZUpCorrector> FlipYZ_ZUpCorrector::create()
{
    return std::make_shared<FlipYZ_ZUpCorrector>();
}

void FlipYZ_ZUpCorrector::convertTransform(Transform &e)
{
    e.position = flip_z(swap_yz(e.position));
    e.rotation = flip_z(swap_yz(e.rotation));
    e.scale = swap_yz(e.scale);

    auto t = e.getType();
    if (t == EntityType::Camera || t == EntityType::Light) {
        const mu::quatf cr = mu::rotate_x(-90.0f * mu::DegToRad);
        e.rotation *= cr;
    }
}

void FlipYZ_ZUpCorrector::convertCamera(Camera &e)
{
    convertTransform(e);

    auto convert = [this](auto& v) {
        return flip_z(swap_yz(v));
    };

    e.view_matrix = convert(e.view_matrix);
    e.proj_matrix = convert(e.proj_matrix);
}

void FlipYZ_ZUpCorrector::convertLight(Light &e)
{
    convertTransform(e);
}

void FlipYZ_ZUpCorrector::convertMesh(Mesh &e)
{
    convertTransform(e);

    auto convert = [this](auto& v) { return flip_z(swap_yz(v)); };

    for (auto& v : e.points) v = convert(v);
    for (auto& v : e.normals) v = convert(v);
    for (auto& v : e.tangents) v = convert(v);
    for (auto& v : e.velocities) v = convert(v);

    for (auto& bone : e.bones) {
        bone->bindpose = convert(bone->bindpose);
    }
    for (auto& bs : e.blendshapes) {
        for (auto& frame : bs->frames) {
            for (auto& v : frame->points) { v = convert(v); }
            for (auto& v : frame->normals) { v = convert(v); }
            for (auto& v : frame->tangents) { v = convert(v); }
        }
    }
}

void FlipYZ_ZUpCorrector::convertCurve(Curve& e) {
    convertTransform(e);

    auto convert = [this](auto& v) { return flip_z(swap_yz(v)); };
    
    for (auto& spline : e.splines) {
        for (auto& v : spline->cos) v = convert(v);
        for (auto& v : spline->handles_left) v = convert(v);
        for (auto& v : spline->handles_right) v = convert(v); 
    }
}

void FlipYZ_ZUpCorrector::convertPoints(Points &e)
{
    convertTransform(e);

    auto convert = [this](auto& v) { return flip_z(swap_yz(v)); };

    for (auto& v : e.points) v = flip_z(swap_yz(v));
    for (auto& v : e.rotations) v = flip_z(swap_yz(v));
    for (auto& v : e.scales) v = swap_yz(v);
}

void FlipYZ_ZUpCorrector::convert(Animation &anim)
{
    auto convert_curve = [&](AnimationCurve& c) {
        if (!c.data_flags.affect_handedness)
            return;

        switch (c.data_type) {
        case Animation::DataType::Float3:
            if (!c.data_flags.ignore_negate)
                c.each<mu::float3>([&](auto& v) { v.value = flip_z(swap_yz(v.value)); });
            else
                c.each<mu::float3>([&](auto& v) { v.value = swap_yz(v.value); });
            break;
        case Animation::DataType::Float4:
            if (!c.data_flags.ignore_negate)
                c.each<mu::float4>([&](auto& v) { v.value = flip_z(swap_yz(v.value)); });
            else
                c.each<mu::float4>([&](auto& v) { v.value = swap_yz(v.value); });
            break;
        case Animation::DataType::Quaternion:
            c.each<mu::quatf>([&](auto& v) { v.value = flip_z(swap_yz(v.value)); });
            if ((anim.entity_type == EntityType::Camera || anim.entity_type == EntityType::Light) && c.name == mskTransformRotation) {
                const mu::quatf cr = mu::rotate_x(-90.0f * mu::DegToRad);
                c.each<mu::quatf>([&](auto& v) {
                    v.value *= cr;
                });
            }
            break;
        default:
            break;
        }
    };

    for (auto& curve : anim.curves) {
        convert_curve(*curve);
    }
}


std::shared_ptr<RotateX_ZUpCorrector> RotateX_ZUpCorrector::create()
{
    return std::make_shared<RotateX_ZUpCorrector>();
}

void RotateX_ZUpCorrector::convertTransform(Transform &e)
{
    if (e.isRoot()) {
        e.position = flip_z(swap_yz(e.position));
        e.rotation = flip_z(swap_yz(e.rotation)) * mu::rotate_x(-90.0f * mu::DegToRad);
        e.scale = swap_yz(e.scale);
    }
}

void RotateX_ZUpCorrector::convertCamera(Camera &e)
{
    convertTransform(e);
}

void RotateX_ZUpCorrector::convertLight(Light &e)
{
    convertTransform(e);
}

void RotateX_ZUpCorrector::convertMesh(Mesh &e)
{
    convertTransform(e);
}

void RotateX_ZUpCorrector::convertCurve(Curve& e)
{
    convertTransform(e);
}

void RotateX_ZUpCorrector::convertPoints(Points &e)
{
    convertTransform(e);
}

void RotateX_ZUpCorrector::convert(Animation &e)
{
    if (!e.isRoot())
        return;
    super::convert(e);
}

void RotateX_ZUpCorrector::convertAnimationCurve(AnimationCurve &c)
{
    if (!c.data_flags.affect_handedness)
        return;

    switch (c.data_type) {
    case Animation::DataType::Float3:
        if (!c.data_flags.ignore_negate)
            c.each<mu::float3>([this](auto& v) { v.value = flip_z(swap_yz(v.value)); });
        else
            c.each<mu::float3>([this](auto& v) { v.value = swap_yz(v.value); });
        break;
    case Animation::DataType::Float4:
        if (!c.data_flags.ignore_negate)
            c.each<mu::float4>([this](auto& v) { v.value = flip_z(swap_yz(v.value)); });
        else
            c.each<mu::float4>([this](auto& v) { v.value = swap_yz(v.value); });
        break;
    case Animation::DataType::Quaternion:
        c.each<mu::quatf>([this](auto& v) { v.value = flip_z(swap_yz(v.value)) * mu::rotate_x(-90.0f * mu::DegToRad); });
        break;
    default:
        break;
    }
}

} // namespace ms
