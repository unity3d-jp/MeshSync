#include "pch.h"
#include "msmodoContext.h"
#include "msmodoUtils.h"


void msmodoContext::TreeNode::clearState()
{
    dst_obj = nullptr;
    dst_anim = nullptr;
}

void msmodoContext::TreeNode::resolveMaterialIDs(const std::vector<ms::MaterialPtr>& materials)
{
    if (material_names.empty() || !dst_obj || dst_obj->getType() != ms::Entity::Type::Mesh)
        return;

    auto& dst = static_cast<ms::Mesh&>(*dst_obj);
    int num_faces = (int)dst.counts.size();

    // material names -> material ids
    dst.material_ids.resize_discard(num_faces);
    for (int fi = 0; fi < num_faces; ++fi) {
        auto mname = material_names[fi];
        int mid = -1;
        auto it = std::lower_bound(materials.begin(), materials.end(), mname,
            [](const ms::MaterialPtr& mp, const char *name) { return std::strcmp(mp->name.c_str(), name) < 0; });
        if (it != materials.end() && (*it)->name == mname)
            mid = (*it)->id;
        dst.material_ids[fi] = mid;
    }
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


void msmodoContext::extractTransformData(TreeNode& n, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
{
    CLxUser_Locator loc(n.item);

    LXtMatrix4 lxmat;
    loc.LocalTransform4(m_ch_read, lxmat);
    mu::float4x4 mat = to_float4x4(lxmat);

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

void msmodoContext::prepare(double time)
{
    super::prepare(time);
}

bool msmodoContext::sendScene(SendScope scope, bool dirty_all)
{
    if (m_sender.isSending()) {
        m_pending_scope = scope;
        return false;
    }
    m_pending_scope = SendScope::None;

    if (dirty_all) {
        m_entity_manager.makeDirtyAll();
    }

    prepare();

    exportMaterials();

    // export entities
    auto do_export = [this](CLxUser_Item& obj) { exportObject(obj); };

    if (scope == SendScope::All) {
        if (m_settings.sync_cameras)
            eachCamera(do_export);
        if (m_settings.sync_lights)
            eachLight(do_export);
        if (m_settings.sync_bones)
            eachMesh([&](CLxUser_Item& obj) { eachBone(obj, do_export); });
        if (m_settings.sync_meshes) {
            eachMesh(do_export);
            eachMeshInstance(do_export);
        }
        m_entity_manager.eraseStaleEntities();
    }

    // send
    kickAsyncSend();
    return true;
}

bool msmodoContext::sendAnimations(SendScope scope)
{
    m_sender.wait();

    prepare();

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
    std::string ptag;
    for (CLxUser_Item i = GetParent(obj); i; i = GetParent(i)) {
        if (m_ch_read.GetString(i, LXsICHAN_MASK_PTAG, ptag))
            break;
    }

    auto ret = ms::Material::create();
    auto& dst = *ret;
    dst.name = ptag;
    dst.id = m_material_ids.getID(obj);
    dst.index = ++m_material_index_seed;

    {
        auto& stdmat = ms::AsStandardMaterial(*ret);

        mu::float4 color{
            (float)m_ch_read.FValue(obj, LXsICHAN_ADVANCEDMATERIAL_DIFFCOL".R"),
            (float)m_ch_read.FValue(obj, LXsICHAN_ADVANCEDMATERIAL_DIFFCOL".G"),
            (float)m_ch_read.FValue(obj, LXsICHAN_ADVANCEDMATERIAL_DIFFCOL".B"),
            1.0f
        };
        stdmat.setColor(color);
    }
    m_material_manager.add(ret);

    return ret;
}

void msmodoContext::exportMaterials()
{
    m_material_index_seed = 0;

    eachMaterial([this](CLxUser_Item& obj) { exportMaterial(obj); });
    m_material_ids.eraseStaleRecords();
    m_material_manager.eraseStaleMaterials();

    // gen material list sorted by name for later resolve
    m_materials = m_material_manager.getAllMaterials();
    std::sort(m_materials.begin(), m_materials.end(),
        [](auto& a, auto& b) { return a->name < b->name; });
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
    else if (obj.IsA(t_meshinst)) {
        exportObject(GetParent(obj));
        n.dst_obj = exportMeshInstance(n);
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

ms::TransformPtr msmodoContext::exportMeshInstance(TreeNode& n)
{
    auto ret = createEntity<ms::Transform>(n);
    n.dst_obj = ret;

    extractTransformData(n, ret->position, ret->rotation, ret->scale, ret->visible);
    enumerateGraph(n.item, LXsGRAPH_MESHINST, [&](CLxUser_Item& g) {
        n.dst_obj->reference = GetPath(g);
    });

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
    ret->rotation = mu::flipY(ret->rotation);

    m_entity_manager.add(n.dst_obj);
    return ret;
}

ms::MeshPtr msmodoContext::exportMesh(TreeNode& n)
{
    auto ret = createEntity<ms::Mesh>(n);
    auto& dst = *ret;
    n.dst_obj = ret;

    extractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible);

    CLxUser_Mesh mesh = m_settings.bake_deformers ? getDeformedMesh(n.item) : getBaseMesh(n.item);

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

    // topology
    {
        n.material_names.resize_discard(num_faces);

        dst.counts.resize_discard(num_faces);
        dst.indices.reserve_discard(num_faces * 4);
        for (int fi = 0; fi < num_faces; ++fi) {
            polygons.SelectByIndex(fi);

            const char *material_name;
            poly_tag.Get(LXi_POLYTAG_MATERIAL, &material_name);
            n.material_names[fi] = material_name;

            uint32_t count;
            polygons.VertexCount(&count);
            dst.counts[fi] = count;

            size_t pos = dst.indices.size();
            dst.indices.resize(pos + count);
            for (uint32_t ci = 0; ci < count; ++ci) {
                LXtPointID pid;
                polygons.VertexByIndex(ci, &pid);
                points.Select(pid);

                uint32_t index;
                points.Index(&index);
                dst.indices[pos + ci] = index;
            }
        }
        num_indices = (int)dst.indices.size();

        // resolving material ids will be done in kickAsyncSend()
    }

    //points
    {
        dst.points.resize_discard(num_points);
        for (int pi = 0; pi < num_points; ++pi) {
            points.SelectByIndex(pi);

            LXtFVector p;
            points.Pos(p);
            dst.points[pi] = to_float3(p);
        }
    }

    // normals
    if (m_settings.sync_normals) {
        auto do_extract_map = [&](const char *name, RawVector<mu::float3>& dst_array) {
            if (LXx_FAIL(mmap.SelectByName(LXi_VMAP_NORMAL, name)))
                return;

            dst_array.resize_discard(dst.indices.size());
            auto *write_ptr = dst_array.data();

            auto mmid = mmap.ID();
            LXtPointID pid;
            mu::float3 v;
            for (int fi = 0; fi < num_faces; ++fi) {
                polygons.SelectByIndex(fi);
                int count = dst.counts[fi];
                for (int ci = 0; ci < count; ++ci) {
                    polygons.VertexByIndex(ci, &pid);
                    if (LXx_FAIL(polygons.MapEvaluate(mmid, pid, &v[0]))) {
                        dst.clear();
                        return;
                    }
                    *(write_ptr++) = v;
                }
            }
        };

        auto do_extract_poly = [&](RawVector<mu::float3>& dst_array) {
            dst_array.resize_discard(dst.indices.size());
            auto *write_ptr = dst_array.data();

            LXtPointID pid;
            for (int fi = 0; fi < num_faces; ++fi) {
                polygons.SelectByIndex(fi);
                auto poly_id = polygons.ID();

                int count = dst.counts[fi];
                for (int ci = 0; ci < count; ++ci) {
                    polygons.VertexByIndex(ci, &pid);
                    points.Select(pid);

                    LXtVector n;
                    points.Normal(poly_id, n);
                    *(write_ptr++) = to_float3(n);
                }
            }
        };

        auto map_names = GetMapNames(mmap, LXi_VMAP_NORMAL);
        if (map_names.size() > 0)
            do_extract_map(map_names[0], dst.normals);
        else
            do_extract_poly(dst.normals);
    }

    // uv
    if (m_settings.sync_uvs) {
        auto do_extract = [&](const char *name, RawVector<mu::float2>& dst_array) {
            if (LXx_FAIL(mmap.SelectByName(LXi_VMAP_TEXTUREUV, name)))
                return;

            dst_array.resize_discard(dst.indices.size());
            auto *write_ptr = dst_array.data();

            auto mmid = mmap.ID();
            LXtPointID pid;
            mu::float2 v;
            for (int fi = 0; fi < num_faces; ++fi) {
                polygons.SelectByIndex(fi);
                int count = dst.counts[fi];
                for (int ci = 0; ci < count; ++ci) {
                    polygons.VertexByIndex(ci, &pid);
                    if (LXx_FAIL(polygons.MapEvaluate(mmid, pid, &v[0]))) {
                        dst.clear();
                        return;
                    }
                    *(write_ptr++) = v;
                }
            }
        };

        auto map_names = GetMapNames(mmap, LXi_VMAP_TEXTUREUV);
        if (map_names.size() > 0)
            do_extract(map_names[0], dst.uv0);
        if (map_names.size() > 1)
            do_extract(map_names[1], dst.uv1);
    }

    // vertex color
    if (m_settings.sync_colors) {
        auto do_extract = [&](const char *name, RawVector<mu::float4>& dst_array) {
            if (LXx_FAIL(mmap.SelectByName(LXi_VMAP_RGBA, name)))
                return;

            dst_array.resize_discard(dst.indices.size());
            auto *write_ptr = dst_array.data();

            auto mmid = mmap.ID();
            LXtPointID pid;
            mu::float4 v;
            for (int fi = 0; fi < num_faces; ++fi) {
                polygons.SelectByIndex(fi);
                int count = dst.counts[fi];
                for (int ci = 0; ci < count; ++ci) {
                    polygons.VertexByIndex(ci, &pid);
                    if (LXx_FAIL(polygons.MapEvaluate(mmid, pid, &v[0]))) {
                        dst.clear();
                        return;
                    }
                    *(write_ptr++) = v;
                }
            }
        };

        auto map_names = GetMapNames(mmap, LXi_VMAP_RGBA);
        if (map_names.size() > 0)
            do_extract(map_names[0], dst.colors);
    }

    // bone weights
    if (!m_settings.bake_deformers && m_settings.sync_bones) {
        auto get_weights = [&](const char *name, RawVector<float>& dst_array) -> bool {
            if (LXx_FAIL(mmap.SelectByName(LXi_VMAP_WEIGHT, name)))
                return false;

            dst_array.resize_discard(dst.points.size());
            auto *write_ptr = dst_array.data();

            auto mmid = mmap.ID();
            float v;
            for (int pi = 0; pi < num_points; ++pi) {
                points.SelectByIndex(pi);
                if (LXx_FAIL(points.MapEvaluate(mmid, &v))) {
                    dst.clear();
                    return false;
                }
                *(write_ptr++) = v;
            }
            return true;
        };

        enumerateGraph(n.item, LXsGRAPH_DEFORMERS, [&](CLxUser_Item& def) {
            if (def.Type() != t_geninf)
                return;

            static uint32_t ch_enable, ch_type, ch_mapname;
            if (ch_enable == 0) {
                def.ChannelLookup(LXsICHAN_GENINFLUENCE_ENABLE, &ch_enable);
                def.ChannelLookup(LXsICHAN_GENINFLUENCE_TYPE, &ch_type);
                def.ChannelLookup(LXsICHAN_GENINFLUENCE_NAME, &ch_mapname);
            }

            int enable, type;
            const char *mapname;
            m_ch_read.Integer(def, ch_enable, &enable);
            m_ch_read.Integer(def, ch_type, &type);
            m_ch_read.String(def, ch_mapname, &mapname);
            if (!mapname)
                return;

            CLxUser_Item effector;
            if (LXx_FAIL(m_deform_service.DeformerDeformationItem(def, effector)) || !effector.IsA(t_locator))
                return;

            auto dst_bone = ms::BoneData::create();
            dst_bone->path = GetPath(effector);
            {
                // bindpose
                CLxUser_Locator loc(effector);
                LXtMatrix4 lxmat;
                loc.WorldTransform4(m_ch_read_setup, lxmat);
                dst_bone->bindpose = mu::invert(to_float4x4(lxmat));
            }
            if (get_weights(mapname, dst_bone->weights))
                dst.bones.push_back(dst_bone);
        });

        if (!dst.bones.empty()) {
            dst.refine_settings.flags.apply_local2world = 1;
            dst.refine_settings.local2world = dst.toMatrix();
        }
    }

    // morph
    if (!m_settings.bake_deformers && m_settings.sync_blendshapes) {
        auto get_delta = [&](const char *name, RawVector<mu::float3>& dst_array) -> bool {
            if (LXx_FAIL(mmap.SelectByName(LXi_VMAP_MORPH, name)))
                return false;

            dst_array.resize_discard(dst.points.size());
            auto *write_ptr = dst_array.data();

            auto mmid = mmap.ID();
            mu::float3 v;
            for (int pi = 0; pi < num_points; ++pi) {
                points.SelectByIndex(pi);
                if (LXx_FAIL(points.MapEvaluate(mmid, &v[0]))) {
                    dst.clear();
                    return false;
                }
                *(write_ptr++) = v;
            }
            return true;
        };

        enumerateGraph(n.item, LXsGRAPH_DEFORMERS, [&](CLxUser_Item& def) {
            if (def.Type() != t_morph)
                return;

            static uint32_t ch_enable, ch_strength, ch_mapname;
            if (ch_enable == 0) {
                def.ChannelLookup(LXsICHAN_MORPHDEFORM_ENABLE, &ch_enable);
                def.ChannelLookup(LXsICHAN_MORPHDEFORM_STRENGTH, &ch_strength);
                def.ChannelLookup(LXsICHAN_MORPHDEFORM_MAPNAME, &ch_mapname);
            }

            int enable;
            double strength;
            const char *mapname;
            m_ch_read.Integer(def, ch_enable, &enable);
            m_ch_read.Double(def, ch_strength, &strength);
            m_ch_read.String(def, ch_mapname, &mapname);
            if (!mapname)
                return;

            auto dst_bs = ms::BlendShapeData::create();
            dst_bs->name = GetName(def);
            dst_bs->weight = (float)(strength * 100.0);

            auto dst_bsf = ms::BlendShapeFrameData::create();
            dst_bs->frames.push_back(dst_bsf);
            dst_bsf->weight = 100.0f;
            if (get_delta(mapname, dst_bsf->points))
                dst.blendshapes.push_back(dst_bs);
        });
    }

    dst.setupFlags();
    dst.refine_settings.flags.make_double_sided = m_settings.make_double_sided;
    dst.refine_settings.flags.gen_tangents = 1;
    dst.refine_settings.flags.swap_faces = 1;

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

    auto do_export = [this](CLxUser_Item& obj) { exportAnimation(obj); };

    if (m_settings.sync_cameras)
        eachCamera(do_export);
    if (m_settings.sync_lights)
        eachLight(do_export);
    if (m_settings.sync_bones)
        eachMesh([&](CLxUser_Item& obj) { eachBone(obj, do_export); });
    if (m_settings.sync_meshes) {
        eachMesh(do_export);
        eachMeshInstance(do_export);
    }

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
        last.value = mu::flipY(last.value);
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
    ms::parallel_for_each(m_tree_nodes.begin(), m_tree_nodes.end(), [this](auto& kvp) {
        auto& node = kvp.second;
        node.resolveMaterialIDs(m_materials);
        node.material_names.clear();
    });

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
