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
    MeshSyncClient3dsMax::getInstance().onRepaint();
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
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeAdded(n);
    });
}

void msmaxNodeCallback::Deleted(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeDeleted(n);
    });
}

void msmaxNodeCallback::LinkChanged(NodeKeyTab & nodes)
{
    MeshSyncClient3dsMax::getInstance().onSceneUpdated();
}

void msmaxNodeCallback::ModelStructured(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeUpdated(n);
    });
}
void msmaxNodeCallback::GeometryChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeUpdated(n);
    });
}
void msmaxNodeCallback::TopologyChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeUpdated(n);
    });
}
void msmaxNodeCallback::MappingChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeUpdated(n);
    });
}
void msmaxNodeCallback::ExtentionChannelChanged(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeUpdated(n);
    });
}
void msmaxNodeCallback::ModelOtherEvent(NodeKeyTab& nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeUpdated(n);
    });
}

void msmaxNodeCallback::ControllerOtherEvent(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeUpdated(n);
    });
}

void msmaxNodeCallback::HideChanged(NodeKeyTab & nodes)
{
    EachNode(nodes, [](INode *n) {
        MeshSyncClient3dsMax::getInstance().onNodeUpdated(n);
    });
    // send immediately
    MeshSyncClient3dsMax::getInstance().update();
}


msmaxTimeChangeCallback & msmaxTimeChangeCallback::getInstance()
{
    static msmaxTimeChangeCallback s_instance;
    return s_instance;
}

void msmaxTimeChangeCallback::TimeChanged(TimeValue t)
{
    MeshSyncClient3dsMax::getInstance().onTimeChanged();
}

