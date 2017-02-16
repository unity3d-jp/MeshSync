#include "pch.h"
#include "MayaUtils.h"

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
    return GetPath(MDagPath::getAPathTo(node));
}

MObject GetTransform(MDagPath path)
{
    return path.transform();
}
MObject GetTransform(MObject node)
{
    return GetTransform(MDagPath::getAPathTo(node));
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

MObject FindMesh(MObject node)
{
    auto path = MDagPath::getAPathTo(node);
    if (path.extendToShape() == MS::kSuccess) {
        auto shape = path.node();
        if (shape.hasFn(MFn::kMesh)) {
            return shape;
        }
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
