#pragma once

// actually, this callback draws nothing.
// just catch repaint callback and use it as idle callback (max seems don't have idle callback)
class msmaxViewportDisplayCallback : public ViewportDisplayCallback
{
public:
    static msmaxViewportDisplayCallback& getInstance();

    void Display(TimeValue t, ViewExp *vpt, int flags) override;
    void GetViewportRect(TimeValue t, ViewExp *vpt, Rect *rect) override;
    BOOL Foreground() override;
};

class msmaxNodeCallback : public INodeEventCallback
{
public:
    static msmaxNodeCallback& getInstance();

    void Added(NodeKeyTab& nodes) override;
    void Deleted(NodeKeyTab& nodes) override;
    void LinkChanged(NodeKeyTab& nodes) override;
    void HierarchyOtherEvent(NodeKeyTab& nodes) override;

    void ModelStructured(NodeKeyTab& nodes) override;
    void GeometryChanged(NodeKeyTab& nodes) override;
    void TopologyChanged(NodeKeyTab& nodes) override;
    void MappingChanged(NodeKeyTab& nodes) override;
    void ExtentionChannelChanged(NodeKeyTab& nodes) override;
    void ModelOtherEvent(NodeKeyTab& nodes) override;

    void ControllerOtherEvent(NodeKeyTab& nodes) override; // transform change callback
    void HideChanged(NodeKeyTab& nodes) override;
};

class msmaxTimeChangeCallback : public TimeChangeCallback
{
public:
    static msmaxTimeChangeCallback& getInstance();

    void TimeChanged(TimeValue t) override;
};


