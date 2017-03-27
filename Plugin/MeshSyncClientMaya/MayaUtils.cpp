#include "pch.h"
#include "MayaUtils.h"
#include "MeshSyncClientMaya.h"

template<class Body>
static void EachChild(MObject node, const Body& body)
{
    MFnDagNode fn = node;
    auto num_children = fn.childCount();
    for (uint32_t i = 0; i < num_children; ++i) {
        body(fn.child(i));
    }
}

bool IsVisible(MObject node)
{
    MFnDagNode dag = node;
    auto vis = dag.findPlug("visibility");
    bool visible = false;
    vis.getValue(visible);
    return visible;
}

std::string GetPath(MDagPath path)
{
    std::string ret = path.fullPathName().asChar();
    std::replace(ret.begin(), ret.end(), '|', '/');
    return ret;
}
std::string GetPath(MObject node)
{
    return GetPath(GetDagPath(node));
}

MDagPath GetDagPath(MObject node)
{
    return MDagPath::getAPathTo(node);
}

MObject GetTransform(MDagPath path)
{
    return path.transform();
}
MObject GetTransform(MObject node)
{
    return GetTransform(GetDagPath(node));
}

MObject GetShape(MObject node)
{
    auto path = GetDagPath(node);
    path.extendToShape();
    return path.node();
}

MObject FindMesh(MObject node)
{
    auto path = GetDagPath(node);
    if (path.extendToShape() == MS::kSuccess) {
        auto shape = path.node();
        if (shape.hasFn(MFn::kMesh)) {
            return shape;
        }
    }
    return MObject();
}

MObject FindSkinCluster(MObject node)
{
    if (!node.hasFn(MFn::kMesh)) {
        node = FindMesh(GetTransform(node));
    }

    MItDependencyGraph it(node, MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
    if (!it.isDone()) {
        return it.currentItem();
    }
    return MObject();
}

MObject FindBlendShape(MObject node)
{
    if (!node.hasFn(MFn::kMesh)) {
        node = FindMesh(GetTransform(node));
    }

    MItDependencyGraph it(node, MFn::kBlendShape, MItDependencyGraph::kUpstream);
    if (!it.isDone()) {
        return it.currentItem();
    }
    return MObject();
}

MObject FindOrigMesh(MObject node)
{
    MObject ret;
    EachChild(node, [&](MObject child) {
        if (child.hasFn(MFn::kMesh)) {
            MFnMesh fn = child;
            if (ret.isNull() || fn.isIntermediateObject()) {
                ret = child;
            }
        }
    });
    return ret;
}

MObject FindInputMesh(const MFnGeometryFilter& gf, const MDagPath& path)
{
    MObjectArray geoms;
    gf.getInputGeometry(geoms);
    for (uint32_t i = 0; i < geoms.length(); ++i) {
        auto geom = geoms[i];
        mscTrace("FindInputMesh(): %s & %s\n", GetDagPath(geom).fullPathName().asChar(), path.fullPathName().asChar());
        if (geom.hasFn(MFn::kMesh) && GetDagPath(geom) == path) {
            return geom;
        }
    }
    return MObject();
}

MObject FindOutputMesh(const MFnGeometryFilter& gf, const MDagPath& path)
{
    MObjectArray geoms;
    gf.getOutputGeometry(geoms);
    for (uint32_t i = 0; i < geoms.length(); ++i) {
        auto geom = geoms[i];
        if (geom.hasFn(MFn::kMesh) && GetDagPath(geom) == path) {
            return geom;
        }
    }

    return MObject();
}

bool JointGetSegmentScaleCompensate(MObject joint)
{
    const MFnIkJoint fn_joint = joint;
    auto plug = fn_joint.findPlug("segmentScaleCompensate");
    return plug.asBool();
}

bool JointGetInverseScale(MObject joint, mu::float3& dst)
{
    const MFnIkJoint fn_joint = joint;
    auto plug = fn_joint.findPlug("inverseScale");
    if (plug.isNull()) { return false; }

    dst = mu::float3{
        (float)plug.child(0).asDouble(),
        (float)plug.child(1).asDouble(),
        (float)plug.child(2).asDouble()
    };
    return true;
}

float ToSeconds(MTime t)
{
    t.setUnit(MTime::kSeconds);
    return (float)t.value();
}

MTime ToMTime(float seconds)
{
    MTime ret;
    ret.setUnit(MTime::kSeconds);
    ret.setValue(seconds);
    return ret;
}


static void DumpPlugInfo(MPlug plug, std::string indent)
{
    uint32_t num_elements = 0;
    uint32_t num_children= 0;

    char array_info[64] = "";
    if (plug.isArray()) {
        num_elements = plug.numElements();
        sprintf(array_info, "[%d]", num_elements);
    }
    mscTrace("%splug %s%s\n", indent.c_str(), plug.name().asChar(), array_info);
    if (plug.isCompound()) {
        auto indent2 = indent + " ";
        num_children = plug.numChildren();
        if (plug.isArray()) {
            for (uint32_t i = 0; i < num_children; ++i) {
                DumpPlugInfo(plug.elementByPhysicalIndex(0).child(i), indent2);
            }
        }
        else {
            for (uint32_t i = 0; i < num_children; ++i) {
                DumpPlugInfo(plug.child(i), indent2);
            }
        }
    }
}

void DumpPlugInfo(MPlug plug)
{
    DumpPlugInfo(plug, "");
    mscTrace("\n");
}




bool GetAnimationCurve(MFnAnimCurve& dst, MPlug& src)
{
    MObjectArray atb;
    MAnimUtil::findAnimation(src, atb);
    if (atb.length() > 0) {
        dst.setObject(atb[0]);
        return true;
    }
    return false;
}

void DeternineTimeRange(float& time_begin, float& time_end, const MFnAnimCurve& curve)
{
    if (curve.object().isNull()) { return; }

    int num_keys = curve.numKeys();
    if (num_keys == 0) { return; }

    float t1 = ToSeconds(curve.time(0));
    float t2 = ToSeconds(curve.time(num_keys > 0 ? num_keys - 1 : 0));
    if (std::isnan(time_begin)) {
        time_begin = t1;
        time_end = t2;
    }
    else {
        time_begin = std::min(time_begin, t1);
        time_end = std::max(time_end, t2);
    }
};

void GatherTimes(RawVector<float>& dst, const MFnAnimCurve& curve)
{
    if (curve.object().isNull()) { return; }

    int num_keys = curve.numKeys();
    size_t pos = dst.size();
    dst.resize(pos + num_keys);
    for (int i = 0; i < num_keys; ++i) {
        dst[pos + i] = ToSeconds(curve.time(i));
    }
};

template<class ValueType, class Assign>
void GatherSamples(
    RawVector<ms::TVP<ValueType>>& dst, MFnAnimCurve& curve,
    const RawVector<float>& time_samples, const Assign& assign)
{
    if (curve.object().isNull()) { return; }
    assert(dst.size() == time_samples.size());

    auto num_samples = time_samples.size();
    for (size_t i = 0; i < num_samples; ++i) {
        auto& tvp = dst[i];
        float t = time_samples[i];
        tvp.time = t;
        assign(tvp.value, curve.evaluate(ToMTime(t)));
    }
}

RawVector<float> BuildTimeSamples(const std::initializer_list<MFnAnimCurve*>& cvs, int samples_per_seconds)
{
    const float time_resolution = 1.0f / (float)samples_per_seconds;
    float time_begin = std::numeric_limits<float>::quiet_NaN();
    float time_end = std::numeric_limits<float>::quiet_NaN();
    float time_range = 0.0f;

    // build time range
    for (auto& cv : cvs) {
        DeternineTimeRange(time_begin, time_end, *cv);
    }
    time_range = time_end - time_begin;


    // build time samples
    RawVector<float> keyframe_times;
    RawVector<float> sample_times;
    for (auto& cv : cvs) {
        GatherTimes(keyframe_times, *cv);
    }
    if (cvs.size() > 1) {
        std::sort(keyframe_times.begin(), keyframe_times.end());
        keyframe_times.erase(std::unique(keyframe_times.begin(), keyframe_times.end()), keyframe_times.end());
    }

    int num_samples = int(time_range / time_resolution);
    sample_times.resize(num_samples - 1);
    for (int i = 1; i < num_samples; ++i) {
        sample_times[i - 1] = (time_resolution * i) + time_begin;
    }

    RawVector<float> times;
    times.resize(keyframe_times.size() + sample_times.size());
    std::merge(keyframe_times.begin(), keyframe_times.end(), sample_times.begin(), sample_times.end(), times.begin());
    times.erase(std::unique(times.begin(), times.end()), times.end());
    return times;
}


void ConvertAnimationBool(
    RawVector<ms::TVP<bool>>& dst,
    bool default_value, MPlug& pb, int samples_per_seconds)
{
    if (pb.isNull()) { return; }

    MFnAnimCurve acv;
    if (!GetAnimationCurve(acv, pb)) { return; }

    auto time_samples = BuildTimeSamples({ ptr(acv) }, samples_per_seconds);
    dst.resize(time_samples.size(), { 0.0f, default_value });
    GatherSamples(dst, acv, time_samples, [](bool& v, double s) { v = s > 0.0; });
}

void ConvertAnimationFloat(
    RawVector<ms::TVP<float>>& dst,
    float default_value, MPlug& pb, int samples_per_seconds)
{
    if (pb.isNull()) { return; }

    MFnAnimCurve acv;
    if (!GetAnimationCurve(acv, pb)) { return; }

    auto time_samples = BuildTimeSamples({ ptr(acv) }, samples_per_seconds);
    dst.resize(time_samples.size(), { 0.0f, default_value });
    GatherSamples(dst, acv, time_samples, [](float& v, double s) { v = (float)s; });
}

void ConvertAnimationFloat3(
    RawVector<ms::TVP<mu::float3>>& dst,
    const mu::float3& default_value, MPlug& px, MPlug& py, MPlug& pz, int samples_per_seconds)
{
    if (px.isNull() && py.isNull() && pz.isNull()) { return; }

    MFnAnimCurve acx, acy, acz;
    int num_valid_curves = 0;
    if (GetAnimationCurve(acx, px)) { ++num_valid_curves; }
    if (GetAnimationCurve(acy, py)) { ++num_valid_curves; }
    if (GetAnimationCurve(acz, pz)) { ++num_valid_curves; }
    if (num_valid_curves == 0) { return; }

    auto time_samples = BuildTimeSamples({ ptr(acx), ptr(acy), ptr(acz) }, samples_per_seconds);
    dst.resize(time_samples.size(), { 0.0f, default_value });
    GatherSamples(dst, acx, time_samples, [](mu::float3& v, double s) { v.x = (float)s; });
    GatherSamples(dst, acy, time_samples, [](mu::float3& v, double s) { v.y = (float)s; });
    GatherSamples(dst, acz, time_samples, [](mu::float3& v, double s) { v.z = (float)s; });
}

void ConvertAnimationFloat4(
    RawVector<ms::TVP<mu::float4>>& dst,
    const mu::float4& default_value, MPlug& px, MPlug& py, MPlug& pz, MPlug& pw, int samples_per_seconds)
{
    if (px.isNull() && py.isNull() && pz.isNull() && pw.isNull()) { return; }

    MFnAnimCurve acx, acy, acz, acw;
    int num_valid_curves = 0;
    if (GetAnimationCurve(acx, px)) { ++num_valid_curves; }
    if (GetAnimationCurve(acy, py)) { ++num_valid_curves; }
    if (GetAnimationCurve(acz, pz)) { ++num_valid_curves; }
    if (GetAnimationCurve(acw, pw)) { ++num_valid_curves; }
    if (num_valid_curves == 0) { return; }

    auto time_samples = BuildTimeSamples({ ptr(acx), ptr(acy), ptr(acz), ptr(acw) }, samples_per_seconds);
    dst.resize(time_samples.size(), { 0.0f, default_value });
    GatherSamples(dst, acx, time_samples, [](mu::float4& v, double s) { v.x = (float)s; });
    GatherSamples(dst, acy, time_samples, [](mu::float4& v, double s) { v.y = (float)s; });
    GatherSamples(dst, acz, time_samples, [](mu::float4& v, double s) { v.z = (float)s; });
    GatherSamples(dst, acw, time_samples, [](mu::float4& v, double s) { v.w = (float)s; });
}
