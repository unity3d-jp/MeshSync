#include "pch.h"
#include "msmodoInterface.h"
#include "msmodoUtils.h"

void msmodoInterface::prepare(double time)
{
    if (time == std::numeric_limits<double>::infinity())
        time = m_selection_service.GetTime();

    m_layer_service.SetScene(0);
    m_layer_service.Scene(m_current_scene);
    m_current_scene.GetChannels(m_ch_read, time);
    m_current_scene.GetSetupChannels(m_ch_read_setup);

    if (t_locator == 0) {
        t_locator = m_scene_service.ItemType(LXsITYPE_LOCATOR);
        t_camera = m_scene_service.ItemType(LXsITYPE_CAMERA);
        t_light = m_scene_service.ItemType(LXsITYPE_LIGHT);
        t_mesh = m_scene_service.ItemType(LXsITYPE_MESH);

        t_geninf = m_scene_service.ItemType(LXsITYPE_GENINFLUENCE);
        t_morph = m_scene_service.ItemType(LXsITYPE_MORPHDEFORM);
    }
}

void msmodoInterface::enumerateGraph(CLxUser_Item& item, const char *graph_name, const std::function<void(CLxUser_Item&)>& body)
{
    CLxUser_SceneGraph scene_graph;
    CLxUser_ItemGraph  item_graph;
    m_current_scene.GetGraph(graph_name, scene_graph);
    item_graph.set(scene_graph);

    unsigned num_elements = item_graph.Reverse(item);
    for (unsigned ti = 0; ti < num_elements; ++ti) {
        CLxUser_Item element;
        if (item_graph.Reverse(item, ti, element)) {
            body(element);
        }
    }
}

#define EachObjectImpl(Type)\
    static const auto ttype = m_scene_service.ItemType(Type);\
    uint32_t num_objects;\
    m_current_scene.ItemCount(ttype, &num_objects);\
    CLxUser_Item item;\
    for (uint32_t im = 0; im < num_objects; ++im) {\
        m_current_scene.ItemByIndex(ttype, im, item);\
        body(item);\
    }

void msmodoInterface::eachMaterial(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(LXsITYPE_ADVANCEDMATERIAL);
}
void msmodoInterface::eachLight(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(LXsITYPE_LIGHT);
}
void msmodoInterface::eachCamera(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(LXsITYPE_CAMERA);
}
void msmodoInterface::eachMesh(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(LXsITYPE_MESH);
}
void msmodoInterface::eachDeformer(const std::function<void(CLxUser_Item&)>& body)
{
    EachObjectImpl(LXsITYPE_DEFORM);
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
    enumerateGraph(item, LXsGRAPH_DEFORMERS, [&](CLxUser_Item& def) {
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
