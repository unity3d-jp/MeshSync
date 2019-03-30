#include "pch.h"
#include "MeshSyncClientModo.h"
#include "msmodoUtils.h"


void TreeNode::clearState()
{
    dst_obj = nullptr;
    dst_anim = nullptr;
}


msmodoContext& msmodoContext::getInstance()
{
    static msmodoContext s_instance;
    return s_instance;
}

msmodoContext::msmodoContext()
{
}

msmodoContext::~msmodoContext()
{
}

msmodoSettings& msmodoContext::getSettings()
{
    return m_settings;
}

void msmodoContext::update()
{
}

void msmodoContext::eachMaterial(const std::function<void(CLxUser_Item&)>& body)
{
    static const auto ttype = m_scene_service.ItemType(LXsITYPE_ADVANCEDMATERIAL);

    uint32_t num_objects;
    m_current_scene.ItemCount(ttype, &num_objects);

    CLxUser_Item item;
    for (uint32_t im = 0; im < num_objects; ++im) {
        m_current_scene.ItemByIndex(ttype, im, item);
        body(item);
    }
}

void msmodoContext::eachLight(const std::function<void(CLxUser_Item&)>& body)
{
    static const auto ttype = m_scene_service.ItemType(LXsITYPE_LIGHT);

    uint32_t num_objects;
    m_current_scene.ItemCount(ttype, &num_objects);

    CLxUser_Item item;
    for (uint32_t im = 0; im < num_objects; ++im) {
        m_current_scene.ItemByIndex(ttype, im, item);
        body(item);
    }
}

void msmodoContext::eachCamera(const std::function<void(CLxUser_Item&)>& body)
{
    static const auto ttype = m_scene_service.ItemType(LXsITYPE_CAMERA);

    uint32_t num_objects;
    m_current_scene.ItemCount(ttype, &num_objects);

    CLxUser_Item item;
    for (uint32_t im = 0; im < num_objects; ++im) {
        m_current_scene.ItemByIndex(ttype, im, item);
        body(item);
    }
}

void msmodoContext::eachMesh(const std::function<void(CLxUser_Item&, CLxUser_Mesh&)>& body)
{
    static const auto ttype = m_scene_service.ItemType(LXsITYPE_MESH);

    uint32_t num_objects;
    m_current_scene.ItemCount(ttype, &num_objects);

    CLxUser_Item item;
    CLxUser_MeshFilter mesh_filter;
    CLxUser_Mesh mesh;
    for (uint32_t im = 0; im < num_objects; ++im) {
        m_current_scene.ItemByIndex(ttype, im, item);

        if (m_chan_read.Object(item, LXsICHAN_MESH_MESH, mesh_filter)) {
            if (mesh_filter.GetMesh(mesh))
                body(item, mesh);
        }
    }
}

static void ExtractTransformData()
{

}

bool msmodoContext::sendScene(SendScope scope, bool dirty_all)
{
    if (m_sender.isSending()) {
        m_pending_scope = scope;
        return false;
    }
    m_pending_scope = SendScope::None;

    m_layer_service.SetScene(0);
    m_layer_service.Scene(m_current_scene);

    m_current_scene.GetChannels(m_chan_read, m_selection_service.GetTime());

    eachMaterial([this](CLxUser_Item& obj) {
        auto name = GetName(obj);
        mu::float4 color {
            (float)m_chan_read.FValue(obj, LXsICHAN_ADVANCEDMATERIAL_DIFFCOL".R"),
            (float)m_chan_read.FValue(obj, LXsICHAN_ADVANCEDMATERIAL_DIFFCOL".G"),
            (float)m_chan_read.FValue(obj, LXsICHAN_ADVANCEDMATERIAL_DIFFCOL".B"),
            1.0f
        };
        msLogInfo("ok\n");
    });

    eachCamera([this](CLxUser_Item& obj) {
    });

    eachLight([this](CLxUser_Item& obj) {
    });

    eachMesh([this](CLxUser_Item& obj, CLxUser_Mesh& mesh) {
        CLxUser_Locator locator(obj);

        CLxUser_StringTag poly_tag;
        CLxUser_Polygon polygons;
        CLxUser_Point points;
        mesh.GetPolygons(polygons);
        mesh.GetPoints(points);
        poly_tag.set(polygons);

        int num_faces = mesh.NPolygons();
        int num_indices = 0;
        int num_points = mesh.NPoints();

        RawVector<int> dst_counts, dst_indices;
        RawVector<mu::float3> dst_points;
        std::string path = GetPath(obj);

        dst_counts.resize_discard(num_faces);
        dst_indices.reserve_discard(num_faces * 4);
        for (int fi = 0; fi < num_faces; ++fi) {
            polygons.SelectByIndex(fi);

            const char *material_name = nullptr;
            poly_tag.Get(LXi_POLYTAG_MATERIAL, &material_name);

            uint32_t count;
            polygons.VertexCount(&count);
            dst_counts[fi] = count;

            size_t pos = dst_indices.size();
            dst_indices.resize(pos + count);
            for (uint32_t ci = 0; ci < count; ++ci) {
                LXtPointID pid;
                polygons.VertexByIndex(ci, &pid);
                points.Select(pid);

                uint32_t index;
                points.Index(&index);
                dst_indices[pos + ci] = index;
            }
        }
        num_indices = (int)dst_indices.size();

        dst_points.resize_discard(num_points);
        for (int pi = 0; pi < num_points; ++pi) {
            points.SelectByIndex(pi);

            LXtFVector p;
            points.Pos(p);
            dst_points[pi] = to_float3(p);
        }

        msLogInfo("ok\n");
    });

    return true;
}

bool msmodoContext::sendAnimations(SendScope scope)
{
    m_sender.wait();

    if (exportAnimations(scope) > 0) {
        kickAsyncSend();
        return true;
    }
    else {
        return false;
    }
}

bool msmodoContext::recvScene()
{
    // todo
    return false;
}


// 
// component export
// 

ms::TransformPtr msmodoContext::exportObject(CLxUser_Item& obj, bool force)
{
    if (!obj.test())
        return nullptr;

    auto& n = m_tree_nodes[obj];
    if (n.dst_obj)
        return n.dst_obj;

    static const auto t_locator = m_scene_service.ItemType(LXsITYPE_LOCATOR);
    static const auto t_camera = m_scene_service.ItemType(LXsITYPE_CAMERA);
    static const auto t_light = m_scene_service.ItemType(LXsITYPE_LIGHT);
    static const auto t_mesh = m_scene_service.ItemType(LXsITYPE_MESH);

    if (obj.IsA(t_camera)) {
        exportObject(GetParent(obj), true);
        n.dst_obj = exportCamera(n);
    }
    else if (obj.IsA(t_light)) {
        exportObject(GetParent(obj), true);
        n.dst_obj = exportLight(n);
    }
    else if (obj.IsA(t_mesh)) {
        exportObject(GetParent(obj), true);
        n.dst_obj = exportMesh(n);
    }
    else {
        exportObject(GetParent(obj), true);
        n.dst_obj = exportTransform(n);
    }

    return n.dst_obj;
}

ms::TransformPtr msmodoContext::exportTransform(TreeNode& node)
{
    auto ret = ms::Transform::create();
    node.dst_obj = ret;

    //CLxUser_Scene      scene(SceneObject());
    //CLxUser_SceneGraph sceneGraph;
    //CLxUser_ItemGraph  itemGraph;
    //scene.GetGraph(LXsGRAPH_XFRMCORE, sceneGraph);
    //itemGraph.set(sceneGraph);

    //unsigned transformCount = itemGraph.Reverse(item);
    //for (unsigned transformIndex = 0; transformIndex < transformCount; ++transformIndex) {
    //    CLxUser_Item transform;
    //    if (itemGraph.Reverse(item, transformIndex, transform)) {
    //        SetItem(transform);

    //        const char *xformType = ItemType();
    //        if (LXTypeMatch(xformType, LXsITYPE_SCALE)) {
    //            // scale
    //            LXtVector scale;
    //            ChanXform(LXsICHAN_SCALE_SCL, scale);

    //        }
    //        else if (LXTypeMatch(xformType, LXsITYPE_ROTATION)) {
    //            // Rotation
    //            LXtVector rotate;
    //            ChanXform(LXsICHAN_ROTATION_ROT, rotate);
    //            int axis[3];
    //            buildAxisOrder(ChanInt(LXsICHAN_ROTATION_ORDER), axis);

    //        }
    //        else if (LXTypeMatch(xformType, LXsITYPE_TRANSLATION)) {
    //            // Translation
    //            LXtVector	translate;
    //            ChanXform(LXsICHAN_TRANSLATION_POS, translate);

    //        }
    //    }
    //}

    return ret;
}

ms::CameraPtr msmodoContext::exportCamera(TreeNode& node)
{
    auto ret = ms::Camera::create();
    node.dst_obj = ret;

    return ret;
}

ms::LightPtr msmodoContext::exportLight(TreeNode& node)
{
    auto ret = ms::Light::create();
    node.dst_obj = ret;

    return ret;
}

ms::MeshPtr msmodoContext::exportMesh(TreeNode& node)
{
    auto ret = ms::Mesh::create();
    node.dst_obj = ret;

    return ret;
}


// 
// animation export
// 

int msmodoContext::exportAnimations(SendScope scope)
{
    m_animations.clear();
    m_animations.push_back(ms::AnimationClip::create());

    eachCamera([this](CLxUser_Item& obj) {
        exportAnimation(obj, true);
    });
    eachLight([this](CLxUser_Item& obj) {
        exportAnimation(obj, true);
    });
    eachMesh([this](CLxUser_Item& obj, CLxUser_Mesh& mesh) {
        exportAnimation(obj, true);
    });

    // todo:

    return 0;
}

bool msmodoContext::exportAnimation(CLxUser_Item& obj, bool force)
{
    if (!obj.test())
        return nullptr;

    auto& n = m_tree_nodes[obj];
    if (n.dst_anim)
        return true;

    static const auto t_locator = m_scene_service.ItemType(LXsITYPE_LOCATOR);
    static const auto t_camera = m_scene_service.ItemType(LXsITYPE_CAMERA);
    static const auto t_light = m_scene_service.ItemType(LXsITYPE_LIGHT);
    static const auto t_mesh = m_scene_service.ItemType(LXsITYPE_MESH);

    if (obj.IsA(t_camera)) {
        exportAnimation(GetParent(obj), true);
        n.dst_anim = ms::CameraAnimation::create();
        n.anim_extractor = &msmodoContext::extractCameraAnimationData;
    }
    else if (obj.IsA(t_light)) {
        exportAnimation(GetParent(obj), true);
        n.dst_anim = ms::LightAnimation::create();
        n.anim_extractor = &msmodoContext::extractLightAnimationData;
    }
    else if (obj.IsA(t_mesh)) {
        exportAnimation(GetParent(obj), true);
        n.dst_anim = ms::MeshAnimation::create();
        n.anim_extractor = &msmodoContext::extractMeshAnimationData;
    }
    else {
        exportAnimation(GetParent(obj), true);
        n.dst_anim = ms::TransformAnimation::create();
        n.anim_extractor = &msmodoContext::extractTransformAnimationData;
    }

    if (n.dst_anim != nullptr) {
        n.dst_anim->path = n.path;
        m_anim_nodes.push_back(&n);
        return true;
    }
    return false;
}

void msmodoContext::extractTransformAnimationData(TreeNode& node)
{

}

void msmodoContext::extractCameraAnimationData(TreeNode& node)
{

}

void msmodoContext::extractLightAnimationData(TreeNode& node)
{

}

void msmodoContext::extractMeshAnimationData(TreeNode& node)
{

}


void msmodoContext::kickAsyncSend()
{
    // cleanup
    for (auto& kvp : m_tree_nodes)
        kvp.second.clearState();

    // kick async send
    m_sender.on_prepare = [this]() {
        auto& t = m_sender;
        t.client_settings = m_settings.client_settings;
        t.scene_settings.handedness = ms::Handedness::Right;
        t.scene_settings.scale_factor = m_settings.scale_factor;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();
        t.animations = m_animations;

        t.deleted_materials = m_material_manager.getDeleted();
        t.deleted_entities = m_entity_manager.getDeleted();
    };
    m_sender.on_success = [this]() {
        m_material_manager.clearDirtyFlags();
        m_texture_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
        m_animations.clear();
    };
    m_sender.kick();
}
