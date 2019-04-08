#pragma once

class msmodoEventListener;
class msmodoTimerCallbackVisitor;

class msmodoInterface
{
public:
    msmodoInterface();
    virtual ~msmodoInterface();

    virtual void prepare();
    void startTimer();

    // events
    virtual void onItemAdd(CLxUser_Item& item);
    virtual void onItemRemove(CLxUser_Item& item);
    virtual void onItemUpdate(CLxUser_Item& item);
    virtual void onTreeRestructure();
    virtual void onTimeChange();
    virtual void onIdle();

    // time: inf -> current time
    void setChannelReadTime(double time = std::numeric_limits<double>::infinity());

    template<class Body> void eachChild(CLxUser_Item& item, const Body& body);

    template<class Body> void enumerateItemGraphR(CLxUser_Item& item, const char *graph_name, const Body& body);
    template<class Body> void enumerateItemGraphF(CLxUser_Item& item, const char *graph_name, const Body& body);
    template<class Body> void enumerateChannelGraphR(CLxUser_Item& item, int channel, const char *graph_name, const Body& body);

    template<class Body> void eachObject(LXtItemType type, const Body& body);
    template<class Body> void eachMaterial(const Body& body);
    template<class Body> void eachLight(const Body& body);
    template<class Body> void eachCamera(const Body& body);
    template<class Body> void eachMesh(const Body& body);
    template<class Body> void eachMeshInstance(const Body& body);
    template<class Body> void eachReplicator(const Body& body);
    template<class Body> void eachBone(CLxUser_Item& item, const Body& body);

    template<class Body> void eachDeformer(CLxUser_Item& item, const Body& body);
    template<class Body> void eachSkinDeformer(CLxUser_Item& item, const Body& body);
    template<class Body> void eachMorphDeformer(CLxUser_Item& item, const Body& body);

    template<class Body> void eachImageMap(CLxUser_Item& material_mask, const Body& body);

    CLxUser_Mesh getBaseMesh(CLxUser_Item& mesh_obj);
    CLxUser_Mesh getDeformedMesh(CLxUser_Item& mesh_obj);
    const char* getImageFilePath(CLxUser_Item& image);

    // Body: [] (CLxUser_Item &geom, const mu::float4x4& matrix) -> void
    template<class Body> void eachReplica(CLxUser_Item& item, const Body& body);

    std::tuple<double, double> getTimeRange();

    void dbgDumpItem(CLxUser_Item item);

public:
    LXtItemType tMaterial = 0,
                tLocator, tCamera, tLight, tMesh, tMeshInst, tReplicator,
                tLightMaterial, tPointLight, tDirectionalLight, tSpotLight, tAreaLight,
                tDeform, tGenInf, tMorph,
                tImageMap, tTextureLayer, tVideoStill;

    CLxUser_SceneService     m_svc_scene;
    CLxUser_SelectionService m_svc_selection;
    CLxUser_LayerService     m_svc_layer;
    CLxUser_MeshService      m_svc_mesh;
    CLxUser_DeformerService  m_svc_deform;
    CLxUser_ListenerService  m_svc_listener;
    CLxUser_LogService       m_svc_log;
    CLxUser_PlatformService  m_svc_platform;

    CLxUser_Scene m_current_scene;
    CLxUser_ChannelRead m_ch_read;
    CLxUser_ChannelRead m_ch_read_setup;
    CLxUser_ItemGraph m_shadeloc_graph;

    msmodoEventListener *m_event_listener = nullptr;
    msmodoTimerCallbackVisitor *m_timer_callback = nullptr;
};


class mdmodoSkinDeformer
{
public:
    mdmodoSkinDeformer(msmodoInterface& ifs, CLxUser_Item& item);
    bool isEnabled();
    int getInfluenceType();
    const char* getMapName();
    CLxUser_Item getEffector();

private:
    static uint32_t ch_enable, ch_type, ch_mapname;
    msmodoInterface& m_ifs;
    CLxUser_Item& m_item;
};


class mdmodoMorphDeformer
{
public:
    mdmodoMorphDeformer(msmodoInterface& ifs, CLxUser_Item& item);
    bool isEnabled();
    float getWeight(); // 0 - 100
    const char* getMapName();

private:
    static uint32_t ch_enable, ch_strength, ch_mapname;
    msmodoInterface& m_ifs;
    CLxUser_Item& m_item;
};



template<class Body>
inline void msmodoInterface::eachChild(CLxUser_Item& item, const Body& body)
{
    CLxUser_Item child;
    uint32_t n;
    item.SubCount(&n);
    for (uint32_t i = 0; i < n; ++i) {
        item.SubByIndex(i, child);
        body(child);
    }
}

template<class Body>
inline void msmodoInterface::enumerateItemGraphR(CLxUser_Item& item, const char *graph_name, const Body& body)
{
    CLxUser_SceneGraph scene_graph;
    if (!m_current_scene.GetGraph(graph_name, scene_graph))
        return;
    CLxUser_ItemGraph item_graph(scene_graph);
    if (!item_graph)
        return;

    uint32_t num = item_graph.Reverse(item);
    for (uint32_t ti = 0; ti < num; ++ti) {
        CLxUser_Item element;
        if (item_graph.Reverse(item, ti, element)) {
            body(element);
        }
    }
}

template<class Body>
inline void msmodoInterface::enumerateItemGraphF(CLxUser_Item& item, const char *graph_name, const Body& body)
{
    CLxUser_SceneGraph scene_graph;
    if (!m_current_scene.GetGraph(graph_name, scene_graph))
        return;
    CLxUser_ItemGraph item_graph(scene_graph);
    if (!item_graph)
        return;

    uint32_t num;
    item_graph.FwdCount(item, &num);
    for (uint32_t ti = 0; ti < num; ++ti) {
        CLxUser_Item element;
        if (item_graph.Forward(item, ti, element)) {
            body(element);
        }
    }
}

template<class Body>
inline void msmodoInterface::enumerateChannelGraphR(CLxUser_Item& item, int channel, const char *graph_name, const Body& body)
{
    CLxUser_SceneGraph scene_graph;
    if (!m_current_scene.GetGraph(graph_name, scene_graph))
        return;
    CLxUser_ChannelGraph channel_graph(scene_graph);
    if (!channel_graph)
        return;

    uint32_t num;
    channel_graph.RevCount(item, channel, &num);
    for (uint32_t ti = 0; ti < num; ++ti) {
        CLxUser_Item element;
        int och;
        if (channel_graph.RevByIndex(item, channel, ti, element, &och)) {
            body(element);
        }
    }
}


template<class Body>
inline void msmodoInterface::eachObject(LXtItemType type, const Body& body)
{
    uint32_t num_objects;
    m_current_scene.ItemCount(type, &num_objects);
    CLxUser_Item item;
    for (uint32_t im = 0; im < num_objects; ++im) {
        m_current_scene.ItemByIndex(type, im, item);
        body(item);
    }
}
template<class Body> inline void msmodoInterface::eachMaterial(const Body& body) { eachObject(tMaterial, body); }
template<class Body> inline void msmodoInterface::eachLight(const Body& body) { eachObject(tLight, body); }
template<class Body> inline void msmodoInterface::eachCamera(const Body& body) { eachObject(tCamera, body); }
template<class Body> inline void msmodoInterface::eachMesh(const Body& body) { eachObject(tMesh, body); }
template<class Body> inline void msmodoInterface::eachMeshInstance(const Body& body) { eachObject(tMeshInst, body); }
template<class Body> inline void msmodoInterface::eachReplicator(const Body& body) { eachObject(tReplicator, body); }

template<class Body>
inline void msmodoInterface::eachBone(CLxUser_Item& item, const Body& body)
{
    eachSkinDeformer(item, [&](CLxUser_Item& def) {
        CLxUser_Item effector;
        if (LXx_OK(m_svc_deform.DeformerDeformationItem(def, effector)) && effector.IsA(tLocator))
            body(effector);
    });
}

template<class Body>
inline void msmodoInterface::eachDeformer(CLxUser_Item& item, const Body& body)
{
    enumerateItemGraphR(item, LXsGRAPH_DEFORMERS, body);
}

template<class Body>
inline void msmodoInterface::eachSkinDeformer(CLxUser_Item& item, const Body& body)
{
    eachDeformer(item, [&](CLxUser_Item& def) {
        if (def.Type() == tGenInf)
            body(def);
    });
}

template<class Body>
inline void msmodoInterface::eachMorphDeformer(CLxUser_Item& item, const Body& body)
{
    eachDeformer(item, [&](CLxUser_Item& def) {
        if (def.Type() == tMorph)
            body(def);
    });
}

template<class Body>
inline void msmodoInterface::eachImageMap(CLxUser_Item& material_mask, const Body& body)
{
    eachChild(material_mask, [&](CLxUser_Item& child) {
        if (child.Type() == tImageMap)
            body(child);
    });
}



template<class Body>
class ReplicaVisitor : public CLxImpl_AbstractVisitor
{
public:
    ReplicaVisitor(msmodoInterface *s, CLxUser_ReplicatorEnumerator *e, const Body& body)
        : m_self(s), m_enumerator(e), m_body(body) {}

    LxResult Evaluate() override
    {
        CLxUser_Item replica;
        if (LXx_OK(m_enumerator->Item(replica))) {
            LXtVector pos;
            LXtMatrix ori;
            m_enumerator->Position(pos);
            m_enumerator->Orientation(ori);

            auto matrix = to_float4x4(ori);
            (mu::float3&)matrix[3] = to_float3(pos);

            m_body(replica, matrix);
        }
        return LXe_OK;
    }

    msmodoInterface *m_self;
    CLxUser_ReplicatorEnumerator *m_enumerator;
    const Body& m_body;
};

template<class Body>
inline void msmodoInterface::eachReplica(CLxUser_Item& item, const Body& body)
{
    CLxUser_ReplicatorEnumerator enumerator;
    m_svc_scene.GetReplicatorEnumerator(item, enumerator);
    if (enumerator) {
        ReplicaVisitor<Body> visitor(this, &enumerator, body);
        enumerator.Enum(&visitor, m_ch_read, true);
    }
}
