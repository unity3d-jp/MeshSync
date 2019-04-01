#include "pch.h"
#include "msmodoInterface.h"
#include "msmodoUtils.h"

void msmodoInterface::prepare()
{
    m_layer_service.SetScene(0);
    m_layer_service.Scene(m_current_scene);
    m_current_scene.GetChannels(m_ch_read, m_selection_service.GetTime());
    m_current_scene.GetSetupChannels(m_ch_read_setup);

    if (tMaterial == 0) {
        tMaterial = m_scene_service.ItemType(LXsITYPE_ADVANCEDMATERIAL);

        tLocator = m_scene_service.ItemType(LXsITYPE_LOCATOR);
        tCamera = m_scene_service.ItemType(LXsITYPE_CAMERA);
        tLight = m_scene_service.ItemType(LXsITYPE_LIGHT);
        tMesh = m_scene_service.ItemType(LXsITYPE_MESH);
        tMeshInst = m_scene_service.ItemType(LXsITYPE_MESHINST);

        tLightMaterial = m_scene_service.ItemType(LXsITYPE_LIGHTMATERIAL);
        tPointLight = m_scene_service.ItemType(LXsITYPE_POINTLIGHT);
        tDirectionalLight = m_scene_service.ItemType(LXsITYPE_SUNLIGHT);
        tSpotLight = m_scene_service.ItemType(LXsITYPE_SPOTLIGHT);
        tAreaLight = m_scene_service.ItemType(LXsITYPE_AREALIGHT);

        tDeform = m_scene_service.ItemType(LXsITYPE_DEFORM);
        tGenInf = m_scene_service.ItemType(LXsITYPE_GENINFLUENCE);
        tMorph = m_scene_service.ItemType(LXsITYPE_MORPHDEFORM);
    }
}

void msmodoInterface::setChannelReadTime(double time)
{
    if (time == std::numeric_limits<double>::infinity())
        time = m_selection_service.GetTime();
    m_current_scene.GetChannels(m_ch_read, time);
}

void msmodoInterface::enumerateItemGraphR(CLxUser_Item& item, const char *graph_name, const std::function<void(CLxUser_Item&)>& body)
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

void msmodoInterface::enumerateItemGraphF(CLxUser_Item& item, const char *graph_name, const std::function<void(CLxUser_Item&)>& body)
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

void msmodoInterface::enumerateChannelGraphR(CLxUser_Item& item, int channel, const char *graph_name, const std::function<void(CLxUser_Item&)>& body)
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


#define EachObjectImpl(Type)\
    uint32_t num_objects;\
    m_current_scene.ItemCount(Type, &num_objects);\
    CLxUser_Item item;\
    for (uint32_t im = 0; im < num_objects; ++im) {\
        m_current_scene.ItemByIndex(Type, im, item);\
        body(item);\
    }

void msmodoInterface::eachMaterial(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(tMaterial);
}
void msmodoInterface::eachLight(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(tLight);
}
void msmodoInterface::eachCamera(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(tCamera);
}
void msmodoInterface::eachMesh(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(tMesh);
}
void msmodoInterface::eachMeshInstance(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(tMeshInst);
}
void msmodoInterface::eachDeformer(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(tDeform);
}
#undef EachObjectImpl

void msmodoInterface::eachMesh(CLxUser_Item& deformer, const std::function<void(CLxUser_Item&)>& body)
{
    CLxUser_Item mesh;
    uint32_t n = 0;
    m_deform_service.MeshCount(deformer, &n);
    for (uint32_t i = 0; i < n; ++i) {
        if (LXx_OK(m_deform_service.MeshByIndex(deformer, i, mesh)))
            body(mesh);
    }
}

void msmodoInterface::eachBone(CLxUser_Item& item, const std::function<void(CLxUser_Item&)>& body)
{
    enumerateItemGraphR(item, LXsGRAPH_DEFORMERS, [&](CLxUser_Item& def) {
        static const auto ttype = m_scene_service.ItemType(LXsITYPE_GENINFLUENCE);
        if (def.Type() != ttype)
            return;

        static const auto tloc = m_scene_service.ItemType(LXsITYPE_LOCATOR);
        CLxUser_Item effector;
        if (LXx_OK(m_deform_service.DeformerDeformationItem(def, effector)) && effector.IsA(tloc))
            body(effector);
    });
}

CLxUser_Mesh msmodoInterface::getBaseMesh(CLxUser_Item& obj)
{
    static const auto ttype = m_scene_service.ItemType(LXsITYPE_MESH);

    CLxUser_Mesh mesh;
    CLxUser_MeshFilter meshfilter;
    CLxUser_MeshFilterIdent meshfilter_ident;
    if (m_ch_read.Object(obj, LXsICHAN_MESH_MESH, meshfilter)) {
        if (meshfilter_ident.copy(meshfilter))
            meshfilter_ident.GetMesh(LXs_MESHFILTER_BASE, mesh);
    }
    return mesh;
}

CLxUser_Mesh msmodoInterface::getDeformedMesh(CLxUser_Item& obj)
{
    static const auto ttype = m_scene_service.ItemType(LXsITYPE_MESH);

    CLxUser_Mesh mesh;
    CLxUser_MeshFilter meshfilter;
    if (m_ch_read.Object(obj, LXsICHAN_MESH_MESH, meshfilter)) {
        meshfilter.GetMesh(mesh);
    }
    return mesh;
}

std::tuple<double, double> msmodoInterface::getTimeRange()
{
    double start = 0.0, end = 1.0;
    {
        CLxReadPreferenceValue q;
        q.Query(LXsPREFERENCE_VALUE_ANIMATION_TIME_RANGE_START);
        start = q.GetFloat();
    }
    {
        CLxReadPreferenceValue q;
        q.Query(LXsPREFERENCE_VALUE_ANIMATION_TIME_RANGE_END);
        end = q.GetFloat();
    }
    return { start, end };
}

void msmodoInterface::dbgDumpItem(CLxUser_Item item)
{
    {
        auto path = GetPath(item);
        auto t = item.Type();
        const char *tname;
        m_scene_service.ItemTypeName(t, &tname);
        msLogInfo("%s (%s)\n", path.c_str(), tname);
    }

    uint32_t n;
    item.ChannelCount(&n);
    for (uint32_t i = 0; i < n; ++i) {
        const char *name, *tname;
        uint32_t type;
        item.ChannelName(i, &name);
        item.ChannelType(i, &type);
        m_scene_service.ItemTypeName(type, &tname);
        msLogInfo(" - %s (%s)\n", name, tname);
    }
}
