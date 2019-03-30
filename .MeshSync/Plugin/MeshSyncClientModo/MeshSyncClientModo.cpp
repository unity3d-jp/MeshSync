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

void msmodoContext::enumrateGraph(CLxUser_Item& item, const char *graph_name, const std::function<void(CLxUser_Item&)>& body)
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

void msmodoContext::eachMesh(const std::function<void(CLxUser_Item&)>& body)
{
    static const auto ttype = m_scene_service.ItemType(LXsITYPE_MESH);

    uint32_t num_objects;
    m_current_scene.ItemCount(ttype, &num_objects);

    CLxUser_Item item;
    for (uint32_t im = 0; im < num_objects; ++im) {
        m_current_scene.ItemByIndex(ttype, im, item);
        body(item);
    }
}

CLxUser_Mesh msmodoContext::getMesh(CLxUser_Item& obj)
{
    static const auto ttype = m_scene_service.ItemType(LXsITYPE_MESH);

    CLxUser_Mesh mesh;
    CLxUser_MeshFilter mesh_filter;
    if (m_chan_read.Object(obj, LXsICHAN_MESH_MESH, mesh_filter)) {
        mesh_filter.GetMesh(mesh);
    }
    return mesh;
}

void msmodoContext::extractTransformData(TreeNode& n, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
{
    auto mat = mu::float4x4::identity();
    enumrateGraph(n.item, LXsGRAPH_XFRMCORE, [this, &mat](CLxUser_Item& transform) {
        const char *tname;
        m_scene_service.ItemTypeName(transform.Type(), &tname);

        if (LXTypeMatch(tname, LXsITYPE_TRANSLATION)) {
            mu::float3 t{
                (float)m_chan_read.FValue(transform, LXsICHAN_TRANSLATION_POS ".X"),
                (float)m_chan_read.FValue(transform, LXsICHAN_TRANSLATION_POS ".Y"),
                (float)m_chan_read.FValue(transform, LXsICHAN_TRANSLATION_POS ".Z")
            };
            mat *= mu::translate(t);
        }
        else if (LXTypeMatch(tname, LXsITYPE_SCALE)) {
            mu::float3 s{
                (float)m_chan_read.FValue(transform, LXsICHAN_SCALE_SCL ".X"),
                (float)m_chan_read.FValue(transform, LXsICHAN_SCALE_SCL ".Y"),
                (float)m_chan_read.FValue(transform, LXsICHAN_SCALE_SCL ".Z")
            };
            mat *= mu::scale44(s);
        }
        else if (LXTypeMatch(tname, LXsITYPE_ROTATION)) {
            enum RotationOrder {
                ROTATION_ORDER_XYZ,
                ROTATION_ORDER_XZY,
                ROTATION_ORDER_YXZ,
                ROTATION_ORDER_YZX,
                ROTATION_ORDER_ZXY,
                ROTATION_ORDER_ZYX
            };

            auto rot_order = (RotationOrder)m_chan_read.IValue(transform, LXsICHAN_ROTATION_ORDER);
            mu::float3 rot_euler{
                (float)m_chan_read.FValue(transform, LXsICHAN_ROTATION_ROT ".X"),
                (float)m_chan_read.FValue(transform, LXsICHAN_ROTATION_ROT ".Y"),
                (float)m_chan_read.FValue(transform, LXsICHAN_ROTATION_ROT ".Z")
            };

            mu::quatf r = mu::quatf::identity();
            switch (rot_order) {
            case ROTATION_ORDER_XYZ: r = mu::rotateXYZ(rot_euler); break;
            case ROTATION_ORDER_XZY: r = mu::rotateXZY(rot_euler); break;
            case ROTATION_ORDER_YXZ: r = mu::rotateYXZ(rot_euler); break;
            case ROTATION_ORDER_YZX: r = mu::rotateYZX(rot_euler); break;
            case ROTATION_ORDER_ZXY: r = mu::rotateZXY(rot_euler); break;
            case ROTATION_ORDER_ZYX: r = mu::rotateZYX(rot_euler); break;
            }

            mat *= mu::to_mat4x4(r);
        }
    });

    pos = extract_position(mat);
    rot = extract_rotation(mat);
    scale = extract_scale(mat);
}

void msmodoContext::extractCameraData(TreeNode& n, bool& ortho, float& near_plane, float& far_plane, float& fov,
    float& horizontal_aperture, float& vertical_aperture, float& focal_length, float& focus_distance)
{
    // todo
}

void msmodoContext::extractLightData(TreeNode& n, ms::Light::LightType& type, mu::float4& color, float& intensity, float& spot_angle)
{
    // todo
}


bool msmodoContext::sendScene(SendScope scope, bool dirty_all)
{
    if (m_sender.isSending()) {
        m_pending_scope = scope;
        return false;
    }
    m_pending_scope = SendScope::None;

    // prepare
    m_layer_service.SetScene(0);
    m_layer_service.Scene(m_current_scene);
    m_current_scene.GetChannels(m_chan_read, m_selection_service.GetTime());

    // export materials
    m_material_index_seed = 0;
    eachMaterial([this](CLxUser_Item& obj) { exportMaterial(obj); });
    m_material_ids.eraseStaleRecords();
    m_material_manager.eraseStaleMaterials();

    // export entities
    eachCamera([this](CLxUser_Item& obj) { exportObject(obj); });
    eachLight([this](CLxUser_Item& obj) { exportObject(obj); });
    eachMesh([this](CLxUser_Item& obj) { exportObject(obj); });

    // send
    kickAsyncSend();
    return true;
}

bool msmodoContext::sendAnimations(SendScope scope)
{
    m_sender.wait();

    // prepare
    m_layer_service.SetScene(0);
    m_layer_service.Scene(m_current_scene);
    m_current_scene.GetChannels(m_chan_read, m_selection_service.GetTime());

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

ms::MaterialPtr msmodoContext::exportMaterial(CLxUser_Item& obj)
{
    auto ret = ms::Material::create();
    ret->name = GetName(obj);
    ret->id = m_material_ids.getID(obj);
    ret->index = ++m_material_index_seed;

    {
        auto& stdmat = ms::AsStandardMaterial(*ret);

        mu::float4 color{
            (float)m_chan_read.FValue(obj, LXsICHAN_ADVANCEDMATERIAL_DIFFCOL".R"),
            (float)m_chan_read.FValue(obj, LXsICHAN_ADVANCEDMATERIAL_DIFFCOL".G"),
            (float)m_chan_read.FValue(obj, LXsICHAN_ADVANCEDMATERIAL_DIFFCOL".B"),
            1.0f
        };
        stdmat.setColor(color);
    }
    m_material_manager.add(ret);

    return ret;
}

ms::TransformPtr msmodoContext::exportObject(CLxUser_Item& obj)
{
    if (!obj.test())
        return nullptr;

    auto& n = m_tree_nodes[obj];
    if (n.dst_obj)
        return n.dst_obj;
    n.item = obj;

    auto name = GetName(obj);
    auto path = GetPath(obj);
    if (!n.path.empty() && n.path!=path) {
        // renamed
        m_entity_manager.erase(n.path);
    }
    if (n.index == 0) {
        n.index = ++m_entity_index_seed;
    }
    n.name = name;
    n.path = path;

    static const auto t_camera = m_scene_service.ItemType(LXsITYPE_CAMERA);
    static const auto t_light = m_scene_service.ItemType(LXsITYPE_LIGHT);
    static const auto t_mesh = m_scene_service.ItemType(LXsITYPE_MESH);

    if (obj.IsA(t_camera)) {
        exportObject(GetParent(obj));
        n.dst_obj = exportCamera(n);
    }
    else if (obj.IsA(t_light)) {
        exportObject(GetParent(obj));
        n.dst_obj = exportLight(n);
    }
    else if (obj.IsA(t_mesh)) {
        exportObject(GetParent(obj));
        n.dst_obj = exportMesh(n);
    }
    else {
        exportObject(GetParent(obj));
        n.dst_obj = exportTransform(n);
    }
    return n.dst_obj;
}

template<class T> std::shared_ptr<T> msmodoContext::createEntity(TreeNode& n)
{
    auto ret = T::create();
    auto& dst = *ret;
    dst.path = n.path;
    dst.index = n.index;
    n.dst_obj = ret;
    return ret;
}


ms::TransformPtr msmodoContext::exportTransform(TreeNode& n)
{
    auto ret = createEntity<ms::Transform>(n);
    n.dst_obj = ret;

    extractTransformData(n, ret->position, ret->rotation, ret->scale, ret->visible);

    m_entity_manager.add(n.dst_obj);
    return ret;
}

ms::CameraPtr msmodoContext::exportCamera(TreeNode& n)
{
    auto ret = createEntity<ms::Camera>(n);
    n.dst_obj = ret;

    extractTransformData(n, ret->position, ret->rotation, ret->scale, ret->visible);
    ret->rotation = mu::flipY(ret->rotation);

    m_entity_manager.add(n.dst_obj);
    return ret;
}

ms::LightPtr msmodoContext::exportLight(TreeNode& n)
{
    auto ret = createEntity<ms::Light>(n);
    n.dst_obj = ret;

    extractTransformData(n, ret->position, ret->rotation, ret->scale, ret->visible);
    ret->rotation = mu::flipY(mu::invert(ret->rotation));

    m_entity_manager.add(n.dst_obj);
    return ret;
}

ms::MeshPtr msmodoContext::exportMesh(TreeNode& n)
{
    auto ret = createEntity<ms::Mesh>(n);
    n.dst_obj = ret;

    extractTransformData(n, ret->position, ret->rotation, ret->scale, ret->visible);

    CLxUser_Mesh mesh = getMesh(n.item);

    CLxUser_StringTag poly_tag;
    CLxUser_Polygon polygons;
    CLxUser_Point points;
    CLxUser_MeshMap mmap;
    mesh.GetPolygons(polygons);
    mesh.GetPoints(points);
    mesh.GetMaps(mmap);
    poly_tag.set(polygons);

    int num_faces = mesh.NPolygons();
    int num_indices = 0;
    int num_points = mesh.NPoints();

    auto& dst_counts = ret->counts;
    auto& dst_indices = ret->indices;
    auto& dst_points = ret->points;
    auto& dst_mids = ret->material_ids;
    std::string path = GetPath(n.item);

    // topology
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

    //points
    dst_points.resize_discard(num_points);
    for (int pi = 0; pi < num_points; ++pi) {
        points.SelectByIndex(pi);

        LXtFVector p;
        points.Pos(p);
        dst_points[pi] = to_float3(p);
    }

    // normals
    {
        auto map_names = GetMapNames(mmap, LXi_VMAP_NORMAL);
        if (!map_names.empty() && LXx_OK(mmap.SelectByName(LXi_VMAP_NORMAL, map_names[0])) ) {
            auto mmid = mmap.ID();

            auto& dst_normals = ret->normals;
            dst_normals.resize_discard(dst_indices.size());
            auto *write_ptr = dst_normals.data();

            for (int fi = 0; fi < num_faces; ++fi) {
                polygons.SelectByIndex(fi);

                int count = dst_counts[fi];
                for (int ci = 0; ci < count; ++ci) {
                    LXtPointID pid;
                    polygons.VertexByIndex(ci, &pid);

                    mu::float3 v;
                    if (LXx_FAIL(polygons.MapEvaluate(mmid, pid, &v[0]))) {
                        dst_normals.clear();
                        goto normals_end;
                    }
                    *(write_ptr++) = v;
                }
            }
        normals_end:;
        }
    }

    // uv
    {
        auto extract_uv = [&](const char *name, RawVector<mu::float2>& dst) {
            if (LXx_FAIL(mmap.SelectByName(LXi_VMAP_TEXTUREUV, name)))
                return;
            auto mmid = mmap.ID();

            dst.resize_discard(dst_indices.size());
            auto *write_ptr = dst.data();

            for (int fi = 0; fi < num_faces; ++fi) {
                polygons.SelectByIndex(fi);

                int count = dst_counts[fi];
                for (int ci = 0; ci < count; ++ci) {
                    LXtPointID pid;
                    polygons.VertexByIndex(ci, &pid);

                    mu::float2 v;
                    if (LXx_FAIL(polygons.MapEvaluate(mmid, pid, &v[0]))) {
                        dst.clear();
                        goto uv_end;
                    }
                    *(write_ptr++) = v;
                }
            }
        uv_end:;
        };

        auto map_names = GetMapNames(mmap, LXi_VMAP_TEXTUREUV);
        if (map_names.size() > 0)
            extract_uv(map_names[0], ret->uv0);
        if (map_names.size() > 1)
            extract_uv(map_names[1], ret->uv1);
    }

    ret->setupFlags();
    ret->refine_settings.flags.swap_faces = true;

    m_entity_manager.add(n.dst_obj);
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
        exportAnimation(obj);
    });
    eachLight([this](CLxUser_Item& obj) {
        exportAnimation(obj);
    });
    eachMesh([this](CLxUser_Item& obj) {
        exportAnimation(obj);
    });

    // todo:

    return 0;
}

bool msmodoContext::exportAnimation(CLxUser_Item& obj)
{
    if (!obj.test())
        return nullptr;

    auto& n = m_tree_nodes[obj];
    if (n.dst_anim)
        return true;
    n.item = obj;

    static const auto t_camera = m_scene_service.ItemType(LXsITYPE_CAMERA);
    static const auto t_light = m_scene_service.ItemType(LXsITYPE_LIGHT);
    static const auto t_mesh = m_scene_service.ItemType(LXsITYPE_MESH);

    if (obj.IsA(t_camera)) {
        exportAnimation(GetParent(obj));
        n.dst_anim = ms::CameraAnimation::create();
        n.anim_extractor = &msmodoContext::extractCameraAnimationData;
    }
    else if (obj.IsA(t_light)) {
        exportAnimation(GetParent(obj));
        n.dst_anim = ms::LightAnimation::create();
        n.anim_extractor = &msmodoContext::extractLightAnimationData;
    }
    else if (obj.IsA(t_mesh)) {
        exportAnimation(GetParent(obj));
        n.dst_anim = ms::MeshAnimation::create();
        n.anim_extractor = &msmodoContext::extractMeshAnimationData;
    }
    else {
        exportAnimation(GetParent(obj));
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

void msmodoContext::extractTransformAnimationData(TreeNode& n)
{
    auto& dst = (ms::TransformAnimation&)*n.dst_anim;

    auto pos = mu::float3::zero();
    auto rot = mu::quatf::identity();
    auto scale = mu::float3::one();
    bool vis = true;
    extractTransformData(n, pos, rot, scale, vis);

    float t = m_anim_time;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
    //dst.visible.push_back({ t, vis });
}

void msmodoContext::extractCameraAnimationData(TreeNode& n)
{
    extractTransformAnimationData(n);

    auto& dst = (ms::CameraAnimation&)*n.dst_anim;
    {
        auto& last = dst.rotation.back();
        last.value = mu::flipY(last.value);
    }

    bool ortho;
    float near_plane, far_plane, fov, horizontal_aperture, vertical_aperture, focal_length, focus_distance;
    extractCameraData(n, ortho, near_plane, far_plane, fov, horizontal_aperture, vertical_aperture, focal_length, focus_distance);

    float t = m_anim_time;
    dst.near_plane.push_back({ t, near_plane });
    dst.far_plane.push_back({ t, far_plane });
    dst.fov.push_back({ t, fov });

#if 0
    dst.horizontal_aperture.push_back({ t, horizontal_aperture });
    dst.vertical_aperture.push_back({ t, vertical_aperture });
    dst.focal_length.push_back({ t, focal_length });
    dst.focus_distance.push_back({ t, focus_distance });
#endif
}

void msmodoContext::extractLightAnimationData(TreeNode& n)
{
    extractTransformAnimationData(n);

    auto& dst = (ms::LightAnimation&)*n.dst_anim;
    {
        auto& last = dst.rotation.back();
        last.value = mu::flipY(mu::invert(last.value));
    }

    ms::Light::LightType type;
    mu::float4 color;
    float intensity;
    float spot_angle;
    extractLightData(n, type, color, intensity, spot_angle);

    float t = m_anim_time;
    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });
    if (type == ms::Light::LightType::Spot)
        dst.spot_angle.push_back({ t, spot_angle });
}

void msmodoContext::extractMeshAnimationData(TreeNode& n)
{
    extractTransformAnimationData(n);

    auto& dst = (ms::MeshAnimation&)*n.dst_anim;
    // todo
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
