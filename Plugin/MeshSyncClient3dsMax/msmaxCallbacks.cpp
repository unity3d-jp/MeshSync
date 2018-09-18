#include "pch.h"
#include "MeshSyncClient3dsMax.h"
#include "msmaxCallbacks.h"
#include "msmaxUtils.h"


msmaxViewportDisplayCallback & msmaxViewportDisplayCallback::getInstance()
{
    static msmaxViewportDisplayCallback s_instance;
    return s_instance;
}

void msmaxViewportDisplayCallback::Display(TimeValue t, ViewExp * vpt, int flags)
{
    msmaxInstance().onRepaint();
}

void msmaxViewportDisplayCallback::GetViewportRect(TimeValue t, ViewExp * vpt, Rect * rect)
{
}

BOOL msmaxViewportDisplayCallback::Foreground()
{
    return 0;
}

msmaxNodeCallback & msmaxNodeCallback::getInstance()
{
    static msmaxNodeCallback s_instance;
    return s_instance;
}

void msmaxNodeCallback::Added(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) { msmaxInstance().onNodeAdded(n); });
}

void msmaxNodeCallback::Deleted(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) { msmaxInstance().onNodeDeleted(n); });
}

void msmaxNodeCallback::LinkChanged(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) { msmaxInstance().onNodeLinkChanged(n); });
}

void msmaxNodeCallback::HierarchyOtherEvent(NodeKeyTab & nodes)
{
    msmaxInstance().onSceneUpdated();
}

void msmaxNodeCallback::ModelStructured(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxInstance().onGeometryUpdated(n); });
}
void msmaxNodeCallback::GeometryChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxInstance().onGeometryUpdated(n); });
}
void msmaxNodeCallback::TopologyChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxInstance().onGeometryUpdated(n); });
}
void msmaxNodeCallback::MappingChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxInstance().onGeometryUpdated(n); });
}
void msmaxNodeCallback::ExtentionChannelChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxInstance().onNodeUpdated(n); });
}
void msmaxNodeCallback::ModelOtherEvent(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxInstance().onNodeUpdated(n); });
}

void msmaxNodeCallback::ControllerOtherEvent(NodeKeyTab & nodes)
{
    msmaxInstance().onSceneUpdated();
}

void msmaxNodeCallback::HideChanged(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) { msmaxInstance().onNodeUpdated(n); });
}


msmaxTimeChangeCallback & msmaxTimeChangeCallback::getInstance()
{
    static msmaxTimeChangeCallback s_instance;
    return s_instance;
}

void msmaxTimeChangeCallback::TimeChanged(TimeValue t)
{
    msmaxInstance().onTimeChanged();
}

