#include "pch.h"
#include "msmaxContext.h"
#include "msmaxCallbacks.h"
#include "msmaxUtils.h"


msmaxViewportDisplayCallback & msmaxViewportDisplayCallback::getInstance()
{
    static msmaxViewportDisplayCallback s_instance;
    return s_instance;
}

void msmaxViewportDisplayCallback::Display(TimeValue t, ViewExp * vpt, int flags)
{
    msmaxGetContext().onRepaint();
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
    EachNode(nodes, [](INode *n) { msmaxGetContext().onNodeAdded(n); });
}

void msmaxNodeCallback::Deleted(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) { msmaxGetContext().onNodeDeleted(n); });
}

void msmaxNodeCallback::LinkChanged(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) { msmaxGetContext().onNodeLinkChanged(n); });
}

void msmaxNodeCallback::HierarchyOtherEvent(NodeKeyTab & nodes)
{
    msmaxGetContext().onSceneUpdated();
}

void msmaxNodeCallback::ModelStructured(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxGetContext().onGeometryUpdated(n); });
}
void msmaxNodeCallback::GeometryChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxGetContext().onGeometryUpdated(n); });
}
void msmaxNodeCallback::TopologyChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxGetContext().onGeometryUpdated(n); });
}
void msmaxNodeCallback::MappingChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxGetContext().onGeometryUpdated(n); });
}
void msmaxNodeCallback::ExtentionChannelChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxGetContext().onNodeUpdated(n); });
}
void msmaxNodeCallback::ModelOtherEvent(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) { msmaxGetContext().onNodeUpdated(n); });
}

void msmaxNodeCallback::ControllerOtherEvent(NodeKeyTab & nodes)
{
    msmaxGetContext().onSceneUpdated();
}

void msmaxNodeCallback::HideChanged(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) { msmaxGetContext().onNodeUpdated(n); });
}


msmaxTimeChangeCallback & msmaxTimeChangeCallback::getInstance()
{
    static msmaxTimeChangeCallback s_instance;
    return s_instance;
}

void msmaxTimeChangeCallback::TimeChanged(TimeValue t)
{
    msmaxGetContext().onTimeChanged();
}

