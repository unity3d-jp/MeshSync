#pragma once

class msmodoTimeChangeTracker;

class msmodoInterface
{
public:
    virtual void prepare();
    virtual void onTimeChange(double t);

    // time: inf -> current time
    void setChannelReadTime(double time = std::numeric_limits<double>::infinity());

    template<class Body> void enumerateItemGraphR(CLxUser_Item& item, const char *graph_name, const Body& body);
    template<class Body> void enumerateItemGraphF(CLxUser_Item& item, const char *graph_name, const Body& body);
    template<class Body> void enumerateChannelGraphR(CLxUser_Item& item, int channel, const char *graph_name, const Body& body);

    template<class Body> void eachObject(LXtItemType type, const Body& body);
    template<class Body> void eachMaterial(const Body& body);
    template<class Body> void eachLight(const Body& body);
    template<class Body> void eachCamera(const Body& body);
    template<class Body> void eachMesh(const Body& body);
    template<class Body> void eachMeshInstance(const Body& body);
    template<class Body> void eachBone(CLxUser_Item& item, const Body& body);

    template<class Body> void eachDeformer(CLxUser_Item& item, const Body& body);
    template<class Body> void eachSkinDeformer(CLxUser_Item& item, const Body& body);
    template<class Body> void eachMorphDeformer(CLxUser_Item& item, const Body& body);

    CLxUser_Mesh getBaseMesh(CLxUser_Item& obj);
    CLxUser_Mesh getDeformedMesh(CLxUser_Item& obj);

    std::tuple<double, double> getTimeRange();

    void dbgDumpItem(CLxUser_Item item);

public:
    LXtItemType tMaterial = 0,
                tLocator, tCamera, tLight, tMesh, tMeshInst,
                tLightMaterial, tPointLight, tDirectionalLight, tSpotLight, tAreaLight,
                tDeform, tGenInf, tMorph;

public:
    CLxUser_SceneService     m_scene_service;
    CLxUser_SelectionService m_selection_service;
    CLxUser_LayerService     m_layer_service;
    CLxUser_MeshService      m_mesh_service;
    CLxUser_DeformerService  m_deform_service;
    CLxUser_ListenerService  m_listener_service;
    CLxUser_LogService       m_log_service;

    CLxUser_Scene m_current_scene;
    CLxUser_ChannelRead m_ch_read;
    CLxUser_ChannelRead m_ch_read_setup;

    msmodoTimeChangeTracker *m_time_change_tracker = nullptr;

private:
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

template<class Body>
inline void msmodoInterface::eachBone(CLxUser_Item& item, const Body& body)
{
    eachSkinDeformer(item, [&](CLxUser_Item& def) {
        CLxUser_Item effector;
        if (LXx_OK(m_deform_service.DeformerDeformationItem(def, effector)) && effector.IsA(tLocator))
            body(effector);
    });
}

template<class Body>
inline void msmodoInterface::eachDeformer(CLxUser_Item& item, const Body& body)
{
    enumerateItemGraphR(item, LXsGRAPH_DEFORMERS, body);
}

template<class Body>
void msmodoInterface::eachSkinDeformer(CLxUser_Item& item, const Body& body)
{
    eachDeformer(item, [&](CLxUser_Item& def) {
        if (def.Type() == tGenInf)
            body(def);
    });
}

template<class Body>
void msmodoInterface::eachMorphDeformer(CLxUser_Item& item, const Body& body)
{
    eachDeformer(item, [&](CLxUser_Item& def) {
        if (def.Type() == tMorph)
            body(def);
    });
}
