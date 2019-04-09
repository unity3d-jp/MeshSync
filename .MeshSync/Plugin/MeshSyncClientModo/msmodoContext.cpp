#include "pch.h"
#include "msmodoContext.h"
#include "msmodoUtils.h"


void msmodoContext::TreeNode::clearState()
{
    dst_obj = nullptr;
    dst_anim = nullptr;
    dst_anim_replicas.clear();
}

void msmodoContext::TreeNode::doExtractAnimation(msmodoContext *self)
{
    if (anim_extractor)
        (self->*anim_extractor)(*this);
}

// * run in parallel *
void msmodoContext::TreeNode::resolveMesh(msmodoContext *self)
{
    if (!dst_obj || dst_obj->getType() != ms::Entity::Type::Mesh)
        return;

    if (!material_names.empty()) {
        auto& materials = self->m_materials;
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
    if (!face_types.empty()) {
        // exclude line objects for now
        bool all_curves = true;
        for (auto t : face_types) {
            if (t != LXiPTYP_CURVE && t != LXiPTYP_BEZIER && t != LXiPTYP_LINE && t != LXiPTYP_BSPLINE) {
                all_curves = false;
                break;
            }
        }
        if (all_curves) {
            self->m_entity_manager.eraseThreadSafe(dst_obj);
            return;
        }
    }
}

// * run in parallel *
void msmodoContext::TreeNode::resolveReplicas(msmodoContext *self)
{
    if (replicas.empty() && prev_replicas.empty())
        return;

    // erase from entity manager if the replica no longer exists
    std::sort(replicas.begin(), replicas.end(),
        [](auto& a, auto& b) { return a->path < b->path; });
    for (auto& p : prev_replicas) {
        auto it = std::lower_bound(replicas.begin(), replicas.end(), p, [](auto& r, auto& p) { return r->path < p->path; });
        if (it == replicas.end() || (*it)->path != p->path)
            self->m_entity_manager.eraseThreadSafe(p);
    }

    prev_replicas = replicas;
}

void msmodoContext::TreeNode::eraseFromEntityManager(msmodoContext *self)
{
    auto& manager = self->m_entity_manager;
    manager.erase(path);
    for (auto& r : prev_replicas)
        manager.erase(r->path);

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
    wait();
}

msmodoSettings& msmodoContext::getSettings()
{
    return m_settings;
}

void msmodoContext::wait()
{
    m_sender.wait();
}

void msmodoContext::update()
{
    if (m_pending_scope != SendScope::None) {
        sendScene(m_pending_scope, false);
    }
}

void msmodoContext::onItemAdd(CLxUser_Item& item)
{
    if (m_settings.auto_sync)
        m_pending_scope = SendScope::All;
}

void msmodoContext::onItemRemove(CLxUser_Item& item)
{
    auto it = m_tree_nodes.find(item);
    if (it != m_tree_nodes.end()) {
        it->second.eraseFromEntityManager(this);
        m_tree_nodes.erase(it);

        if (m_settings.auto_sync)
            m_pending_scope = SendScope::All;
    }
}

void msmodoContext::onItemUpdate(CLxUser_Item& item)
{
    if (m_ignore_events)
        return;

    auto it = m_tree_nodes.find(item);
    if (it != m_tree_nodes.end()) {
        it->second.dirty = true;

        if (m_settings.auto_sync && m_pending_scope == SendScope::None)
            m_pending_scope = SendScope::Updated;
    }
    else {
        //dbgDumpItem(item);
    }
}

void msmodoContext::onTreeRestructure()
{
    if (m_settings.auto_sync)
        m_pending_scope = SendScope::All;
}

void msmodoContext::onTimeChange()
{
    if (m_settings.auto_sync)
        sendScene(SendScope::All, false);
}

void msmodoContext::onIdle()
{
    update();
}


void msmodoContext::extractTransformData(TreeNode& n, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
{
    CLxUser_Locator loc(n.item);

    LXtMatrix4 lxmat;
    loc.LocalTransform4(m_ch_read, lxmat);
    mu::float4x4 mat = to_float4x4(lxmat);

    pos = extract_position(mat);
    rot = extract_rotation(mat);
    if (n.item.IsA(tCamera) || n.item.IsA(tLight)) {
        rot = mu::flipY(rot);
        if (n.item.IsA(tLight))
            rot *= mu::rotateZ(180.0f * mu::Deg2Rad);
    }
    scale = extract_scale(mat);
    vis = loc.Visible(m_ch_read) == LXe_TRUE;
}

void msmodoContext::extractCameraData(TreeNode& n, bool& ortho, float& near_plane, float& far_plane, float& fov,
    float& horizontal_aperture, float& vertical_aperture, float& focal_length, float& focus_distance)
{
    static uint32_t ch_proj_type, ch_res_x, ch_res_y, ch_aperture_x, ch_aperture_y, ch_focal_len, ch_focus_dist, ch_clip_dist;
    if (ch_proj_type == 0) {
        n.item.ChannelLookup(LXsICHAN_CAMERA_PROJTYPE, &ch_proj_type);
        n.item.ChannelLookup(LXsICHAN_CAMERA_RESX, &ch_res_x);
        n.item.ChannelLookup(LXsICHAN_CAMERA_RESY, &ch_res_y);
        n.item.ChannelLookup(LXsICHAN_CAMERA_APERTUREX, &ch_aperture_x);
        n.item.ChannelLookup(LXsICHAN_CAMERA_APERTUREY, &ch_aperture_y);
        n.item.ChannelLookup(LXsICHAN_CAMERA_FOCALLEN, &ch_focal_len);
        n.item.ChannelLookup(LXsICHAN_CAMERA_FOCUSDIST, &ch_focus_dist);
        n.item.ChannelLookup(LXsICHAN_CAMERA_CLIPDIST, &ch_clip_dist);
    }

    int proj, res_x, res_y;
    double aperture_x, aperture_y, focal_len, focus_dist;
    m_ch_read.Integer(n.item, ch_proj_type, &proj);
    m_ch_read.Integer(n.item, ch_res_x, &res_x);
    m_ch_read.Integer(n.item, ch_res_y, &res_y);
    m_ch_read.Double(n.item, ch_aperture_x, &aperture_x);
    m_ch_read.Double(n.item, ch_aperture_y, &aperture_y);
    m_ch_read.Double(n.item, ch_focal_len, &focal_len);
    m_ch_read.Double(n.item, ch_focus_dist, &focus_dist);

    horizontal_aperture = (float)aperture_x;
    vertical_aperture = (float)aperture_y;
    focal_length = (float)focal_len;
    focus_distance = (float)focus_dist;

    ortho = proj == 1;
    fov = mu::compute_fov(vertical_aperture, focal_length);

    // disable near/far plane
    near_plane = far_plane = 0.0f;
}

void msmodoContext::extractLightData(TreeNode& n, ms::Light::LightType& type, mu::float4& color, float& intensity, float& range, float& spot_angle)
{
    // light shape specific data
    auto t = n.item.Type();
    if (t == tDirectionalLight) {
        type = ms::Light::LightType::Directional;
    }
    else if (t == tAreaLight) {
        type = ms::Light::LightType::Area;
    }
    else if (t == tSpotLight) {
        type = ms::Light::LightType::Spot;

        static uint32_t ch_radius, ch_cone;
        if (ch_radius == 0) {
            n.item.ChannelLookup(LXsICHAN_SPOTLIGHT_RADIUS, &ch_radius);
            n.item.ChannelLookup(LXsICHAN_SPOTLIGHT_CONE, &ch_cone);
        }
        double radius, cone;
        m_ch_read.Double(n.item, ch_radius, &radius);
        m_ch_read.Double(n.item, ch_cone, &cone);
        range = (float)radius;
        spot_angle = (float)cone * mu::Rad2Deg;
    }
    else if (t == tPointLight) {
        type = ms::Light::LightType::Point;

        static uint32_t ch_radius;
        if (ch_radius == 0) {
            n.item.ChannelLookup(LXsICHAN_POINTLIGHT_RADIUS, &ch_radius);
        }
        double radius;
        m_ch_read.Double(n.item, ch_radius, &radius);
        range = (float)radius;
    }
    else {
        type = ms::Light::LightType::Point;
    }

    // radiance
    {
        static uint32_t ch_radiance;
        if (ch_radiance == 0) {
            n.item.ChannelLookup(LXsICHAN_LIGHT_RADIANCE, &ch_radiance);
        }
        double radiance;
        m_ch_read.Double(n.item, ch_radiance, &radiance);

        // this is purely based on my observation...
        const float RadianceToIntensity = 0.4f;
        intensity = (float)radiance * RadianceToIntensity;
    }

    // color
    CLxUser_Item light_material;
    enumerateItemGraphR(n.item, LXsGRAPH_PARENT, [this, &light_material](CLxUser_Item& o) {
        if (o.Type() == tLightMaterial)
            light_material = o;
    });
    if (light_material) {
        static uint32_t ch_color_r, ch_color_g, ch_color_b;
        if (ch_color_r == 0) {
            light_material.ChannelLookup(LXsICHAN_LIGHTMATERIAL_LIGHTCOL".R", &ch_color_r);
            light_material.ChannelLookup(LXsICHAN_LIGHTMATERIAL_LIGHTCOL".G", &ch_color_g);
            light_material.ChannelLookup(LXsICHAN_LIGHTMATERIAL_LIGHTCOL".B", &ch_color_b);
        }

        double r, g, b;
        m_ch_read.Double(light_material, ch_color_r, &r);
        m_ch_read.Double(light_material, ch_color_g, &g);
        m_ch_read.Double(light_material, ch_color_b, &b);
        color = mu::float4{ (float)r, (float)g, (float)b, 1.0f };
    }
}

void msmodoContext::extractReplicaData(
    TreeNode& n, CLxUser_Item& geom, int nth, const mu::float4x4& matrix,
    std::string& path, mu::float3& pos, mu::quatf& rot, mu::float3& scale)
{
    if (n.dst_obj)
        path = n.dst_obj->path;
    else if (n.dst_anim)
        path = n.dst_anim->path;
    else
        path = GetPath(n.item);
    path += '/';
    path += GetName(geom);

    char buf[64];
    sprintf(buf, " (%d)", nth++);
    path += buf;

    pos = mu::extract_position(matrix);
    rot = mu::extract_rotation(matrix);
    scale = mu::extract_scale(matrix);
}

bool msmodoContext::sendMaterials(bool dirty_all)
{
    if (dirty_all) {
        m_material_manager.setAlwaysMarkDirty(dirty_all);
        m_texture_manager.setAlwaysMarkDirty(dirty_all);
    }
    exportMaterials();

    // send
    kickAsyncSend();
    return true;
}

bool msmodoContext::sendScene(SendScope scope, bool dirty_all)
{
    if (m_sender.isSending()) {
        m_pending_scope = scope;
        return false;
    }
    m_pending_scope = SendScope::None;

    prepare();
    m_entity_manager.setAlwaysMarkDirty(dirty_all);
    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(false); // false because too heavy

    // erase dead entities
    for (auto i = m_tree_nodes.begin(); i != m_tree_nodes.end(); /**/) {
        auto& n = i->second;
        if (!valid(n.item)) {
            m_tree_nodes.erase(i++);
            n.eraseFromEntityManager(this);
        }
        else
            ++i;
    }

    // materials
    exportMaterials();

    // entities
    if (scope == SendScope::All) {
        auto do_export = [this](CLxUser_Item& obj) { exportObject(obj, true); };

        eachCamera(do_export);
        eachLight(do_export);
        eachMesh([&](CLxUser_Item& obj) { eachBone(obj, do_export); });
        eachMesh(do_export);
        eachMeshInstance(do_export);
        eachReplicator(do_export);
    }
    else if (scope == SendScope::Updated) {
        for (auto& kvp : m_tree_nodes) {
            auto& n = kvp.second;
            if (n.dirty)
                exportObject(n.item, false);
        }
    }

    // send
    kickAsyncSend();
    return true;
}

bool msmodoContext::sendAnimations(SendScope scope)
{
    wait();
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
    wait();

    m_svc_log.DebugOut(LXi_DBLOG_NORMAL, "Not implemented yet. sorry!\n");

    // todo
    return false;
}


// 
// component export
// 

ms::MaterialPtr msmodoContext::exportMaterial(CLxUser_Item obj)
{
    CLxUser_Item mask;
    std::string ptagtype;
    std::string ptag;
    for (CLxUser_Item i = GetParent(obj); i; i = GetParent(i)) {
        m_ch_read.GetString(i, LXsICHAN_MASK_PTYP, ptagtype);
        if (ptagtype == "Material") {
            mask = i;
            m_ch_read.GetString(i, LXsICHAN_MASK_PTAG, ptag);
            break;
        }
    }
    if (ptag.empty())
        return nullptr;

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

        if (m_settings.sync_textures) {
            eachImageMap(mask, [&](CLxUser_Item& image) {
                const char *effect;
                if (!m_ch_read.Get(image, LXsICHAN_TEXTURELAYER_EFFECT, &effect))
                    return;

                if (match(effect, LXs_FX_DIFFCOLOR)) {
                    if (auto filename = getImageFilePath(image))
                        stdmat.setColorMap(m_texture_manager.addFile(filename, ms::TextureType::Default));
                }
                else if (
                    match(effect, LXs_FX_LUMICOLOR) ||
                    match(effect, LXs_FX_UE_EMIS) ||
                    match(effect, LXs_FX_UT_EMIS) ||
                    match(effect, LXs_FX_GLTF_EMIS))
                {
                    if (auto filename = getImageFilePath(image))
                        stdmat.setEmissionMap(m_texture_manager.addFile(filename, ms::TextureType::Default));
                }
                else if (
                    match(effect, LXs_FX_BUMP) ||
                    match(effect, LXs_FX_NORMAL) ||
                    match(effect, LXs_FX_UE_NORMAL) ||
                    match(effect, LXs_FX_UT_NORMAL) ||
                    match(effect, LXs_FX_GLTF_NORMAL))
                {
                    if (auto filename = getImageFilePath(image))
                        stdmat.setBumpMap(m_texture_manager.addFile(filename, ms::TextureType::NormalMap));
                }
            });
        }
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


ms::TransformPtr msmodoContext::exportObject(CLxUser_Item obj, bool parent, bool tip)
{
    if (!valid(obj))
        return nullptr;

    auto& n = m_tree_nodes[obj];
    n.dirty = false;
    if (n.dst_obj)
        return n.dst_obj;
    n.item = obj;

    auto name = GetName(obj);
    auto path = GetPath(obj);
    if (!n.path.empty() && n.path != path) {
        // renamed
        n.eraseFromEntityManager(this);
    }
    if (n.index == 0) {
        n.index = ++m_entity_index_seed;
    }
    n.name = name;
    n.path = path;

    auto handle_parent = [&]() {
        if (parent)
            exportObject(GetParent(obj), parent, false);
    };
    auto handle_transform = [&]() {
        handle_parent();
        n.dst_obj = exportTransform(n);
    };

    if (obj.IsA(tCamera)) {
        if (m_settings.sync_cameras) {
            handle_parent();
            n.dst_obj = exportCamera(n);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else if (obj.IsA(tLight)) {
        if (m_settings.sync_lights) {
            handle_parent();
            n.dst_obj = exportLight(n);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else if (obj.IsA(tMesh)) {
        if (m_settings.sync_bones)
            eachBone(obj, [&](CLxUser_Item& bone) { exportObject(bone, true); });

        if (m_settings.sync_meshes) {
            handle_parent();
            n.dst_obj = exportMesh(n);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else if (obj.IsA(tMeshInst)) {
        if (m_settings.sync_mesh_instances) {
            handle_parent();
            n.dst_obj = exportMeshInstance(n);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else if (obj.IsA(tReplicator)) {
        if (m_settings.sync_replicators) {
            handle_parent();
            n.dst_obj = exportReplicator(n);
        }
        else if (!tip && parent)
            handle_transform();
    }
    else {
        // bone or intermediate node
        handle_transform();
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
    auto& dst = *ret;

    extractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible);

    m_entity_manager.add(n.dst_obj);
    return ret;
}

ms::TransformPtr msmodoContext::exportMeshInstance(TreeNode& n)
{
    auto ret = createEntity<ms::Transform>(n);
    n.dst_obj = ret;
    auto& dst = *ret;

    extractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible);
    enumerateItemGraphR(n.item, LXsGRAPH_MESHINST, [&](CLxUser_Item& g) {
        n.dst_obj->reference = GetPath(g);
    });

    m_entity_manager.add(n.dst_obj);
    return ret;
}

ms::CameraPtr msmodoContext::exportCamera(TreeNode& n)
{
    auto ret = createEntity<ms::Camera>(n);
    n.dst_obj = ret;
    auto& dst = *ret;

    extractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible);
    extractCameraData(n, dst.is_ortho, dst.near_plane, dst.far_plane, dst.fov,
        dst.horizontal_aperture, dst.vertical_aperture, dst.focal_length, dst.focus_distance);

    m_entity_manager.add(n.dst_obj);
    return ret;
}

ms::LightPtr msmodoContext::exportLight(TreeNode& n)
{
    auto ret = createEntity<ms::Light>(n);
    n.dst_obj = ret;
    auto& dst = *ret;

    extractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible);
    extractLightData(n, dst.light_type, dst.color, dst.intensity, dst.range, dst.spot_angle);

    m_entity_manager.add(n.dst_obj);
    return ret;
}

ms::MeshPtr msmodoContext::exportMesh(TreeNode& n)
{
    CLxUser_Mesh mesh = m_settings.bake_deformers ? getDeformedMesh(n.item) : getBaseMesh(n.item);
    if (!mesh)
        return nullptr;

    auto ret = createEntity<ms::Mesh>(n);
    auto& dst = *ret;
    n.dst_obj = ret;

    extractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible);

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
        n.face_types.resize_discard(num_faces);
        n.material_names.resize_discard(num_faces);

        dst.counts.resize_discard(num_faces);
        dst.indices.reserve_discard(num_faces * 4);
        for (int fi = 0; fi < num_faces; ++fi) {
            polygons.SelectByIndex(fi);

            polygons.Type(&n.face_types[fi]);

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
            if (!name || LXx_FAIL(mmap.SelectByName(LXi_VMAP_WEIGHT, name)))
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

        eachSkinDeformer(n.item, [&](CLxUser_Item& def) {
            mdmodoSkinDeformer skin(*this, def);

            auto joint = skin.getEffector();
            if (!joint || !joint.IsA(tLocator))
                return;

            auto dst_bone = ms::BoneData::create();
            dst_bone->path = GetPath(joint);
            {
                // bindpose
                CLxUser_Locator loc(joint);
                LXtMatrix4 lxmat;
                loc.WorldTransform4(m_ch_read_setup, lxmat);
                dst_bone->bindpose = mu::invert(to_float4x4(lxmat));
            }
            if (get_weights(skin.getMapName(), dst_bone->weights))
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
            if (!name || LXx_FAIL(mmap.SelectByName(LXi_VMAP_MORPH, name)))
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

        eachMorphDeformer(n.item, [&](CLxUser_Item& def) {
            mdmodoMorphDeformer morph(*this, def);

            auto dst_bs = ms::BlendShapeData::create();
            dst_bs->name = GetName(def);
            dst_bs->weight = morph.getWeight();

            auto dst_bsf = ms::BlendShapeFrameData::create();
            dst_bs->frames.push_back(dst_bsf);
            dst_bsf->weight = 100.0f;
            if (get_delta(morph.getMapName(), dst_bsf->points))
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

ms::TransformPtr msmodoContext::exportReplicator(TreeNode& n)
{
    auto ret = createEntity<ms::Transform>(n);
    n.dst_obj = ret;
    auto& dst = *ret;

    extractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible);
    m_entity_manager.add(n.dst_obj);

    // export replica
    int nth = 0;
    n.replicas.clear();
    eachReplica(n.item, [&](CLxUser_Item& geom, mu::float4x4 matrix) {
        auto p = ms::Transform::create();
        auto& dst = *p;
        dst.reference = GetPath(geom);
        extractReplicaData(n, geom, nth++, matrix, dst.path, dst.position, dst.rotation, dst.scale);

        n.replicas.push_back(p);
        m_entity_manager.add(p);
    });
    return ret;
}



// 
// animation export
// 

int msmodoContext::exportAnimations(SendScope scope)
{
    // create default clip
    m_animations.clear();
    m_animations.push_back(ms::AnimationClip::create());

    // gather target objects
    int num_exported = 0;
    {
        auto do_export = [this, &num_exported](CLxUser_Item& obj) {
            if (exportAnimation(obj))
                ++num_exported;
        };

        if (m_settings.sync_cameras)
            eachCamera(do_export);
        if (m_settings.sync_lights)
            eachLight(do_export);
        if (m_settings.sync_bones)
            eachMesh([&](CLxUser_Item& obj) { eachBone(obj, do_export); });
        if (m_settings.sync_meshes)
            eachMesh(do_export);
        if (m_settings.sync_mesh_instances)
            eachMeshInstance(do_export);
        if (m_settings.sync_replicators)
            eachReplicator(do_export);
    }


    // advance frame and record
    double time_current = m_svc_selection.GetTime();
    double time_start, time_end;
    std::tie(time_start, time_end) = getTimeRange();
    auto interval = (1.0 / std::max(m_settings.animation_sps, 0.01f));

    int reserve_size = int((time_end - time_start) / interval) + 1;
    for (auto& n : m_anim_nodes)
        n->dst_anim->reserve(reserve_size);

    m_ignore_events = true;
    for (double t = time_start;;) {
        m_anim_time = (float)(t - time_start) * m_settings.animation_time_scale;
        setChannelReadTime(t);
        for (auto& n : m_anim_nodes)
            n->doExtractAnimation(this);

        if (t >= time_end)
            break;
        else
            t = std::min(t + interval, time_end);
    }
    setChannelReadTime();
    m_ignore_events = false;

    // cleanup
    m_anim_nodes.clear();

    if (m_settings.reduce_keyframes) {
        // keyframe reduction
        for (auto& clip : m_animations)
            clip->reduction(m_settings.keep_flat_curves);

        // erase empty animation
        m_animations.erase(
            std::remove_if(m_animations.begin(), m_animations.end(), [](ms::AnimationClipPtr& p) { return p->empty(); }),
            m_animations.end());
    }

    if (num_exported == 0)
        m_animations.clear();

    return num_exported;
}


#define DefAnimationExtractor(Type, Extractor)\
    template<> msmodoContext::AnimationExtractor msmodoContext::getAnimationExtractor<Type>() { return &Extractor; }

DefAnimationExtractor(ms::TransformAnimation, msmodoContext::extractTransformAnimationData);
DefAnimationExtractor(ms::CameraAnimation, msmodoContext::extractCameraAnimationData);
DefAnimationExtractor(ms::LightAnimation, msmodoContext::extractLightAnimationData);
DefAnimationExtractor(ms::MeshAnimation, msmodoContext::extractMeshAnimationData);

#undef DefAnimationExtractor

template<class T>
std::shared_ptr<T> msmodoContext::createAnimation(TreeNode& n)
{
    auto ret = T::create();
    auto& dst = *ret;
    dst.path = GetPath(n.item);
    n.dst_anim = ret;
    n.anim_extractor = getAnimationExtractor<T>();
    m_animations.front()->animations.push_back(ret);
    return ret;
}

bool msmodoContext::exportAnimation(CLxUser_Item obj)
{
    if (!obj)
        return false;

    auto& n = m_tree_nodes[obj];
    if (n.dst_anim)
        return false;
    n.item = obj;

    if (obj.IsA(tCamera)) {
        exportAnimation(GetParent(obj));
        n.dst_anim = createAnimation<ms::CameraAnimation>(n);
    }
    else if (obj.IsA(tLight)) {
        exportAnimation(GetParent(obj));
        n.dst_anim = createAnimation<ms::LightAnimation>(n);
    }
    else if (obj.IsA(tMesh)) {
        exportAnimation(GetParent(obj));
        n.dst_anim = createAnimation<ms::MeshAnimation>(n);
    }
    else if (obj.IsA(tReplicator)) {
        exportAnimation(GetParent(obj));
        n.dst_anim = createAnimation<ms::TransformAnimation>(n);
        n.anim_extractor = &msmodoContext::extractReplicatorAnimationData;
    }
    else { // includes MeshInstances
        exportAnimation(GetParent(obj));
        n.dst_anim = createAnimation<ms::TransformAnimation>(n);
    }

    if (n.dst_anim != nullptr) {
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

    ms::Light::LightType type;
    mu::float4 color;
    float intensity;
    float range;
    float spot_angle;
    extractLightData(n, type, color, intensity, range, spot_angle);

    float t = m_anim_time;
    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });
    if (type == ms::Light::LightType::Point || type == ms::Light::LightType::Spot)
        dst.range.push_back({ t, range });
    if (type == ms::Light::LightType::Spot)
        dst.spot_angle.push_back({ t, spot_angle });
}

void msmodoContext::extractMeshAnimationData(TreeNode& n)
{
    extractTransformAnimationData(n);

    auto& dst = (ms::MeshAnimation&)*n.dst_anim;

    // extract blendshape weights
    if (!m_settings.bake_deformers && m_settings.sync_blendshapes) {
        CLxUser_Mesh mesh = getBaseMesh(n.item);
        CLxUser_MeshMap mmap;
        mesh.GetMaps(mmap);


        float t = m_anim_time;
        eachMorphDeformer(n.item, [&](CLxUser_Item& def) {
            mdmodoMorphDeformer morph(*this, def);

            const char *mapname = morph.getMapName();
            if (mapname && LXx_OK(mmap.SelectByName(LXi_VMAP_MORPH, mapname))) {
                auto bsa = dst.findOrCreateBlendshapeAnimation(GetName(def));
                bsa->weight.push_back({ t, morph.getWeight() });
            }
        });
    }
}

void msmodoContext::extractReplicatorAnimationData(TreeNode& n)
{
    extractTransformAnimationData(n);

    // export replicas
    int nth = 0;
    eachReplica(n.item, [&](CLxUser_Item& geom, mu::float4x4 matrix) {
        std::string path;
        auto pos = mu::float3::zero();
        auto rot = mu::quatf::identity();
        auto scale = mu::float3::one();
        extractReplicaData(n, geom, nth++, matrix, path, pos, rot, scale);

        auto &dst_ptr = n.dst_anim_replicas[path];
        if (!dst_ptr) {
            dst_ptr = ms::TransformAnimation::create();
            dst_ptr->path = path;
            m_animations.front()->animations.push_back(dst_ptr);
        }
        auto& dst = static_cast<ms::TransformAnimation&>(*dst_ptr);

        float t = m_anim_time;
        dst.translation.push_back({ t, pos });
        dst.rotation.push_back({ t, rot });
        dst.scale.push_back({ t, scale });
    });
}

void msmodoContext::kickAsyncSend()
{
    // resolve
    ms::parallel_for_each(m_tree_nodes.begin(), m_tree_nodes.end(), [this](auto& kvp) {
        auto& node = kvp.second;
        node.resolveMesh(this);
        node.resolveReplicas(this);
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
