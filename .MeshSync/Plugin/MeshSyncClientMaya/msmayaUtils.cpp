#define _MApiVersion
#include "pch.h"
#include "msmayaUtils.h"
#include "MeshSyncClientMaya.h"

std::string GetName(const MObject& node)
{
    MFnDependencyNode fn_node(node);
    return fn_node.name().asChar();
}
std::string ToString(const MDagPath& path)
{
    std::string ret = path.fullPathName().asChar();
    std::replace(ret.begin(), ret.end(), '|', '/');
    return ret;
}

std::string RemoveNamespace(const std::string& path)
{
    static const std::regex s_remove_head("^([^/]+:)");
    static const std::regex s_remove_leaf("/([^/]+:)");

    auto ret = std::regex_replace(path, s_remove_head, "");
    return std::regex_replace(ret, s_remove_leaf, "/");
}

bool IsVisible(const MObject& node)
{
    Pad<MFnDagNode> dag(node);
    auto vis = dag.findPlug("visibility", true);
    bool visible = true;
    vis.getValue(visible);
    return visible;
}

MDagPath GetParent(const MDagPath & node)
{
    MDagPath ret = node;
    ret.pop(1);
    return ret;
}

bool IsInstance(const MObject& node)
{
    Pad<MFnDagNode> dn(node);
    return dn.isInstanced(false);
}

MObject FindSkinCluster(MObject shape)
{
    MItDependencyGraph it(shape, MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
    if (!it.isDone()) {
        return it.currentItem();
    }
    return MObject();
}

MObject FindBlendShape(MObject shape)
{
    MItDependencyGraph it(shape, MFn::kBlendShape, MItDependencyGraph::kUpstream);
    if (!it.isDone()) {
        return it.currentItem();
    }
    return MObject();
}

MObject FindOrigMesh(const MObject& shape)
{
    MObject ret;
    EachChild(shape, [&](const MObject& child) {
        if (child.hasFn(MFn::kMesh)) {
            MFnMesh fn(child);
            if (ret.isNull() || fn.isIntermediateObject()) {
                ret = child;
            }
        }
    });
    return ret;
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

mu::float4x4 GetPivotMatrix(MObject node)
{
    mu::float4x4 ret = mu::float4x4::identity();
    if (!node.hasFn(MFn::kTransform))
        return ret;

    MFnTransform fn_trans(node);
    auto pivot = fn_trans.rotatePivot(MSpace::kTransform);
    (mu::float3&)ret[3] = to_float3(pivot);
    return ret;
}


#ifdef mscDebug
static void DumpPlugInfoImpl(MPlug plug, std::string indent)
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

void DumpPlugInfoImpl(MPlug plug)
{
    DumpPlugInfoImpl(plug, "");
    mscTrace("\n");
}

void DumpPlugInfoImpl(MFnDependencyNode& fn)
{
    MPlugArray connected;
    fn.getConnections(connected);
    for (uint32_t i = 0; i < connected.length(); ++i) {
        DumpPlugInfoImpl(connected[i], " ");
    }

    uint32_t num_attributes = fn.attributeCount();
    for (uint32_t i = 0; i < num_attributes; ++i) {
        MObject attr = fn.attribute(i);
        MFnAttribute fn_attr(attr);
        mscTrace(" attr %s (%s)\n", fn_attr.name().asChar(), attr.apiTypeStr());
        DumpPlugInfoImpl(fn.findPlug(fn_attr.name(), true), " ");
    }
}
#endif

