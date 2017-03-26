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
