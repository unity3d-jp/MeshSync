#include "pch.h"
#include "msblenContext.h"
#include "msblenUtils.h"


void SyncSettings::validate()
{
    if (!bake_modifiers)
        bake_transform = false;
}

void msblenContext::NodeRecord::clearState()
{
    dst_anim = nullptr;
    anim_extractor = nullptr;
}

void msblenContext::NodeRecord::recordAnimation(msblenContext *_this)
{
    (_this->*anim_extractor)(*dst_anim, obj);
}


void msblenContext::ObjectRecord::clearState()
{
    touched = renamed = false;
    dst = nullptr;
}


msblenContext& msblenContext::getInstance()
{
    static msblenContext s_instance;
    return s_instance;
}

msblenContext::msblenContext()
{
    m_settings.scene_settings.handedness = ms::Handedness::RightZUp;
}

msblenContext::~msblenContext()
{
    // no wait() because it can cause deadlock...
}

SyncSettings& msblenContext::getSettings() { return m_settings; }
const SyncSettings& msblenContext::getSettings() const { return m_settings; }
CacheSettings& msblenContext::getCacheSettings() { return m_cache_settings; }
const CacheSettings& msblenContext::getCacheSettings() const { return m_cache_settings; }

std::vector<Object*> msblenContext::getNodes(ObjectScope scope)
{
    std::vector<Object*> ret;

    auto scene = bl::BScene(bl::BContext::get().scene());
    if (scope == ObjectScope::All) {
        scene.each_objects([&](Object *obj) {
            ret.push_back(obj);
        });
    }
    else if (scope == ObjectScope::Selected) {
        scene.each_selection([&](Object *obj) {
            ret.push_back(obj);
        });
    }
    else if (scope == ObjectScope::Updated) {
        auto bpy_data = bl::BData(bl::BContext::get().data());
        if (bpy_data.objects_is_updated()) {
            scene.each_objects([&](Object *obj) {
                auto bid = bl::BID(obj);
                if (bid.is_updated() || bid.is_updated_data())
                    ret.push_back(obj);
            });
        }
    }

    return ret;
}

int msblenContext::exportTexture(const std::string & path, ms::TextureType type)
{
    return m_texture_manager.addFile(path, type);
}

int msblenContext::getMaterialID(Material *m)
{
#if BLENDER_VERSION >= 280
    if (m && m->id.orig_id)
        m = (Material*)m->id.orig_id;
#endif
    return m_material_ids.getID(m);
}

void msblenContext::exportMaterials()
{
    int midx = 0;
    
    // Blender allows faces to have no material. add dummy material for them.
    {
        auto dst = ms::Material::create();
        dst->id = m_material_ids.getID(nullptr);
        dst->index = midx++;
        dst->name = "Default";
        m_material_manager.add(dst);
    }

    auto bpy_data = bl::BData(bl::BContext::get().data());
    for (auto *mat : bpy_data.materials()) {
        auto ret = ms::Material::create();
        ret->name = get_name(mat);
        ret->id = m_material_ids.getID(mat);
        ret->index = midx++;

        auto& stdmat = ms::AsStandardMaterial(*ret);
        bl::BMaterial bm(mat);
        auto color_src = mat;
        if (bm.use_nodes()) {
#if BLENDER_VERSION < 280
            bl::BMaterial node(bm.active_node_material());
            if (node.ptr()) {
                color_src = node.ptr();
            }
#endif
        }
        stdmat.setColor(mu::float4{ color_src->r, color_src->g, color_src->b, 1.0f });

        // todo: handle texture
#if 0
        if (m_settings.sync_textures) {
            auto export_texture = [this](MTex *mtex, ms::TextureType type) -> int {
                if (!mtex || !mtex->tex || !mtex->tex->ima)
                    return -1;
                return exportTexture(bl::abspath(mtex->tex->ima->name), type);
            };
#if BLENDER_VERSION < 280
            stdmat.setColorMap(export_texture(mat->mtex[0], ms::TextureType::Default));
#endif
        }
#endif

        m_material_manager.add(ret);
    }
    m_material_ids.eraseStaleRecords();
    m_material_manager.eraseStaleMaterials();
}


static const mu::float4x4 g_arm_to_world = mu::float4x4{
    1, 0, 0, 0,
    0, 0,-1, 0,
    0, 1, 0, 0,
    0, 0, 0, 1
};
static const mu::float4x4 g_world_to_arm = mu::float4x4{
    1, 0, 0, 0,
    0, 0, 1, 0,
    0,-1, 0, 0,
    0, 0, 0, 1
};

static inline mu::float4x4 camera_correction(const mu::float4x4& v)
{
    return mu::float4x4{ {
        {-v[0][0],-v[0][1],-v[0][2],-v[0][3]},
        { v[1][0], v[1][1], v[1][2], v[1][3]},
        {-v[2][0],-v[2][1],-v[2][2],-v[2][3]},
        { v[3][0], v[3][1], v[3][2], v[3][3]},
    } };
}


mu::float4x4 msblenContext::getWorldMatrix(const Object *obj)
{
    auto r = bl::BObject(obj).matrix_world();
    if (is_camera(obj) || is_light(obj)) {
        // camera/light correction
        r = camera_correction(r);
    }
    return r;
}

mu::float4x4 msblenContext::getLocalMatrix(const Object *obj)
{
    auto r = bl::BObject(obj).matrix_local();
    if (obj->parent && obj->partype == PARBONE) {
        if (auto bone = find_bone(obj->parent, obj->parsubstr)) {
            r *= mu::translate(mu::float3{ 0.0f, mu::length((mu::float3&)bone->tail - (mu::float3&)bone->head), 0.0f });
            r *= g_world_to_arm;
        }
    }

    if (is_camera(obj) || is_light(obj)) {
        // camera/light correction
        r = camera_correction(r);
    }
    return r;
}

mu::float4x4 msblenContext::getLocalMatrix(const Bone *bone)
{
    auto r = (mu::float4x4&)bone->arm_mat;
    if (auto parent = bone->parent)
        r *= mu::invert((mu::float4x4&)parent->arm_mat);
    else
        r *= g_arm_to_world;
    // todo: armature to world here
    return r;
}

mu::float4x4 msblenContext::getLocalMatrix(const bPoseChannel *pose)
{
    auto r = (mu::float4x4&)pose->pose_mat;
    if (auto parent = pose->parent)
        r *= mu::invert((mu::float4x4&)parent->pose_mat);
    else
        r *= g_arm_to_world;
    // todo: armature to world here
    return r;
}

static void extract_bone_trs(const mu::float4x4& mat, mu::float3& t, mu::quatf& r, mu::float3& s)
{
    mu::extract_trs(mat, t, r, s);
    // armature-space to world-space
    t = mu::swap_yz(mu::flip_z(t));
    r = mu::swap_yz(mu::flip_z(r));
    s = mu::swap_yz(s);
}

void msblenContext::extractTransformData(Object *obj,
    mu::float3& t, mu::quatf& r, mu::float3& s, ms::VisibilityFlags& vis,
    mu::float4x4 *dst_world, mu::float4x4 *dst_local)
{
    vis = { true, visible_in_render(obj), visible_in_viewport(obj) };

    auto local = getLocalMatrix(obj);
    auto world = getWorldMatrix(obj);
    if (dst_world)
        *dst_world = world;
    if (dst_local)
        *dst_local = local;

    if (m_settings.bake_transform) {
        if (is_camera(obj) || is_light(obj)) {
            mu::extract_trs(world, t, r, s);
        }
        else {
            t = mu::float3::zero();
            r = mu::quatf::identity();
            s = mu::float3::one();
        }
    }
    else {
        mu::extract_trs(local, t, r, s);
    }
}

void msblenContext::extractTransformData(Object *src, ms::Transform& dst)
{
    extractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visibility, &dst.world_matrix, &dst.local_matrix);
}

void msblenContext::extractTransformData(const bPoseChannel *src, mu::float3& t, mu::quatf& r, mu::float3& s)
{
    if (m_settings.bake_transform) {
        t = mu::float3::zero();
        r = mu::quatf::identity();
        s = mu::float3::one();
    }
    else {
        extract_bone_trs(getLocalMatrix(src), t, r, s);
    }
}

void msblenContext::extractCameraData(Object *src,
    bool& ortho, float& near_plane, float& far_plane, float& fov,
    float& focal_length, mu::float2& sensor_size, mu::float2& lens_shift)
{
    bl::BCamera cam(src->data);

    // note: fbx exporter seems always export as perspective
    ortho = ((Camera*)src->data)->type == CAM_ORTHO;

    near_plane = cam.clip_start();
    far_plane = cam.clip_end();
    fov = cam.angle_y() * mu::RadToDeg;
    focal_length = cam.lens();
    sensor_size.x = cam.sensor_width();  // in mm
    sensor_size.y = cam.sensor_height(); // in mm

    auto fit = cam.sensor_fit();
    if (fit == CAMERA_SENSOR_FIT_AUTO)
        fit = sensor_size.x > sensor_size.y ? CAMERA_SENSOR_FIT_HOR : CAMERA_SENSOR_FIT_VERT;
    const float aspx = sensor_size.x / sensor_size.y;
    const float aspy = sensor_size.y / sensor_size.x;
    lens_shift.x = cam.shift_x() * (fit == CAMERA_SENSOR_FIT_HOR ? 1.0f : aspy); // 0-1
    lens_shift.y = cam.shift_y() * (fit == CAMERA_SENSOR_FIT_HOR ? aspx : 1.0f); // 0-1
}

void msblenContext::extractLightData(Object *src,
    ms::Light::LightType& ltype, ms::Light::ShadowType& stype, mu::float4& color, float& intensity, float& range, float& spot_angle)
{
#if BLENDER_VERSION < 280
    auto data = (Lamp*)src->data;
    const float energy_to_intensity = 1.0f;
#else
    auto data = (Light*)src->data;
    const float energy_to_intensity = 0.001f;
#endif
    color = (mu::float4&)data->r;
    intensity = data->energy * energy_to_intensity;
    range = data->dist;

    switch (data->type) {
    case LA_SUN:
        ltype = ms::Light::LightType::Directional;
        break;
    case LA_SPOT:
        ltype = ms::Light::LightType::Spot;
        spot_angle = data->spotsize * mu::RadToDeg;
        break;
    case LA_AREA:
        ltype = ms::Light::LightType::Area;
        break;
    default:
        ltype = ms::Light::LightType::Point;
        break;
    }
    stype = (data->mode & 1) ? ms::Light::ShadowType::Soft : ms::Light::ShadowType::None;
}


ms::TransformPtr msblenContext::exportObject(Object *obj, bool parent, bool tip)
{
    if (!obj)
        return nullptr;

    auto& rec = touchRecord(obj);
    if (rec.dst)
        return rec.dst; // already exported

    auto handle_parent = [&]() {
        if (parent)
            exportObject(obj->parent, parent, false);
    };
    auto handle_transform = [&]() {
        handle_parent();
        rec.dst = exportTransform(obj);
    };

    switch (obj->type) {
    case OB_ARMATURE:
    {
        if (!tip || (!m_settings.bake_modifiers && m_settings.sync_bones)) {
            handle_parent();
            rec.dst = exportArmature(obj);
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    case OB_MESH:
    {
        if (!m_settings.bake_modifiers && m_settings.sync_bones) {
            if (auto *arm_mod = (const ArmatureModifierData*)find_modofier(obj, eModifierType_Armature))
                exportObject(arm_mod->object, parent);
        }
        if (m_settings.sync_meshes || (!m_settings.bake_modifiers && m_settings.sync_blendshapes)) {
            handle_parent();
            rec.dst = exportMesh(obj);
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    case OB_FONT:  //
    case OB_CURVE: //
    case OB_SURF:  //
    case OB_MBALL: // these can be converted to mesh
    {
        if (m_settings.sync_meshes && m_settings.curves_as_mesh) {
            handle_parent();
            rec.dst = exportMesh(obj);
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    case OB_CAMERA:
    {
        if (m_settings.sync_cameras) {
            handle_parent();
            rec.dst = exportCamera(obj);
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    case OB_LAMP:
    {
        if (m_settings.sync_lights) {
            handle_parent();
            rec.dst = exportLight(obj);
        }
        else if (!tip && parent)
            handle_transform();
        break;
    }
    default:
    {
        if (get_instance_collection(obj) || (!tip && parent)) {
            handle_parent();
            rec.dst = exportTransform(obj);
        }
        break;
    }
    }

    if (rec.dst) {
        if (get_instance_collection(obj)) {
            DupliGroupContext ctx;
            ctx.group_host = obj;
            ctx.dst = rec.dst;

            exportDupliGroup(obj, ctx);
        }
    }
    return rec.dst;
}

ms::TransformPtr msblenContext::exportTransform(Object *src)
{
    auto ret = ms::Transform::create();
    auto& dst = *ret;
    dst.path = get_path(src);
    extractTransformData(src, dst);
    m_entity_manager.add(ret);
    return ret;
}

ms::TransformPtr msblenContext::exportPose(Object *armature, bPoseChannel *src)
{
    auto ret = ms::Transform::create();
    auto& dst = *ret;
    dst.path = get_path(armature, src->bone);
    extractTransformData(src, dst.position, dst.rotation, dst.scale);
    m_entity_manager.add(ret);
    return ret;
}

ms::TransformPtr msblenContext::exportArmature(Object *src)
{
    auto ret = ms::Transform::create();
    auto& dst = *ret;
    dst.path = get_path(src);
    extractTransformData(src, dst);
    m_entity_manager.add(ret);

    for (auto pose : bl::list_range((bPoseChannel*)src->pose->chanbase.first)) {
        auto bone = pose->bone;
        auto& dst = m_bones[bone];
        dst = exportPose(src, pose);
    }
    return ret;
}

ms::TransformPtr msblenContext::exportReference(Object *src, const DupliGroupContext& ctx)
{
    auto& rec = touchRecord(src);
    if (!rec.dst)
        return nullptr;

    auto local_path = get_path(src);
    auto path = ctx.dst->path + local_path;

    ms::TransformPtr dst;
    auto assign_base_params = [&]() {
        extractTransformData(src, *dst);
        dst->path = path;
        // todo:
        dst->visibility = {};
        dst->world_matrix *= ctx.dst->world_matrix;
    };

    if (is_mesh(src)) {
        if (m_settings.bake_transform) {
            dst = ms::Mesh::create();
            auto& dst_mesh = (ms::Mesh&)*dst;
            auto& src_mesh = (ms::Mesh&)*rec.dst;

            (ms::Transform&)dst_mesh = (ms::Transform&)src_mesh;
            assign_base_params();

            auto do_merge = [this, dst, &dst_mesh, &src_mesh]() {
                dst_mesh.merge(src_mesh);
                if (m_settings.export_cache)
                    dst_mesh.detach();
                dst_mesh.refine_settings = src_mesh.refine_settings;
                dst_mesh.refine_settings.local2world = dst_mesh.world_matrix;
                dst_mesh.refine_settings.flags.local2world = 1;
                m_entity_manager.add(dst);
            };
            if (m_settings.multithreaded)
                // deferred to execute after extracting src mesh data is completed
                m_async_tasks.push_back(std::async(std::launch::deferred, do_merge));
            else
                do_merge();
        }
        else {
            dst = ms::Transform::create();
            assign_base_params();
            dst->reference = local_path;
            m_entity_manager.add(dst);
        }
    }
    else {
        dst = std::static_pointer_cast<ms::Transform>(rec.dst->clone());
        assign_base_params();
        m_entity_manager.add(dst);
    }

    each_child(src, [&](Object *child) {
        exportReference(child, ctx);
    });

    if (get_instance_collection(src)) {
        DupliGroupContext ctx2;
        ctx2.group_host = src;
        ctx2.dst = dst;

        exportDupliGroup(src, ctx2);
    }
    return dst;
}

ms::TransformPtr msblenContext::exportDupliGroup(Object *src, const DupliGroupContext& ctx)
{
    auto group = get_instance_collection(src);
    if (!group)
        return nullptr;

    auto local_path = std::string("/") + (group->id.name + 2);
    auto path = ctx.dst->path + local_path;

    auto dst = ms::Transform::create();
    dst->path = path;
    dst->visibility = { true, visible_in_render(ctx.group_host), visible_in_viewport(ctx.group_host) };

    auto offset_pos = -get_instance_offset(group);
    dst->position = m_settings.bake_transform ? mu::float3::zero() : offset_pos;
    dst->world_matrix = mu::translate(offset_pos) * ctx.dst->world_matrix;
    m_entity_manager.add(dst);

    DupliGroupContext ctx2;
    ctx2.group_host = src;
    ctx2.dst = dst;
    auto gobjects = bl::list_range((CollectionObject*)group->gobject.first);
    for (auto go : gobjects) {
        auto obj = go->ob;
        if (auto t = exportObject(obj, true, false)) {
            bool non_lib = obj->id.lib == nullptr;
            t->visibility = { true, non_lib, non_lib };
        }
        exportReference(obj, ctx2);
    }
    return dst;
}

ms::CameraPtr msblenContext::exportCamera(Object *src)
{
    auto ret = ms::Camera::create();
    auto& dst = *ret;
    dst.path = get_path(src);
    extractTransformData(src, dst);
    extractCameraData(src, dst.is_ortho, dst.near_plane, dst.far_plane, dst.fov, dst.focal_length, dst.sensor_size, dst.lens_shift);
    m_entity_manager.add(ret);
    return ret;
}

ms::LightPtr msblenContext::exportLight(Object *src)
{
    auto ret = ms::Light::create();
    auto& dst = *ret;
    dst.path = get_path(src);
    extractTransformData(src, dst);
    extractLightData(src, dst.light_type, dst.shadow_type, dst.color, dst.intensity, dst.range, dst.spot_angle);
    m_entity_manager.add(ret);
    return ret;
}

ms::MeshPtr msblenContext::exportMesh(Object *src)
{
    // ignore particles
    if (//find_modofier(src, eModifierType_ParticleSystem) ||
        find_modofier(src, eModifierType_ParticleInstance) )
        return nullptr;

    bl::BObject bobj(src);
    Mesh *data = nullptr;
    if (is_mesh(src))
        data = (Mesh*)src->data;
    bool is_editing = false;

    if (m_settings.sync_meshes && data) {
        // check if mesh is dirty
        if (auto edit_mesh = get_edit_mesh(data)) {
            is_editing = true;
            auto bm = edit_mesh->bm;
            if (bm->elem_table_dirty) {
                // mesh is editing and dirty. just add to pending list
                m_pending.insert(src);
                return nullptr;
            }
        }
    }

    auto ret = ms::Mesh::create();
    auto& dst = *ret;
    dst.path = get_path(src);

    // transform
    extractTransformData(src, dst);

    if (m_settings.sync_meshes) {
        bool need_convert = 
            (!is_editing && m_settings.bake_modifiers) || !is_mesh(src);

        if (need_convert) {
#if BLENDER_VERSION >= 280
            if (m_settings.bake_modifiers) {
                auto depsgraph = bl::BContext::get().evaluated_depsgraph_get();
                bobj = (Object*)bl::BID(bobj).evaluated_get(depsgraph);
            }
#endif
            if (Mesh *tmp = bobj.to_mesh()) {
                data = tmp;
#if BLENDER_VERSION < 280
                m_tmp_meshes.push_back(tmp); // baked meshes are need to be deleted manually
#else
                m_meshes_to_clear.push_back(src);
#endif
            }
        }

        // calculate per index normals
        // note: when bake_modifiers is enabled, it is done for baked meshes
        if (data && m_settings.sync_normals && m_settings.calc_per_index_normals) {
            // calc_normals_split() seems can't be multi-threaded. it will cause unpredictable crash...
            // todo: calculate normals by myself to be multi-threaded
            bl::BMesh(data).calc_normals_split();
        }
    }

    if (data) {
        auto task = [this, ret, src, data]() {
            auto& dst = *ret;
            doExtractMeshData(dst, src, data, dst.world_matrix);
            m_entity_manager.add(ret);
        };

        if (m_settings.multithreaded)
            m_async_tasks.push_back(std::async(std::launch::async, task));
        else
            task();
    }
    return ret;
}

void msblenContext::doExtractMeshData(ms::Mesh& dst, Object *obj, Mesh *data, mu::float4x4 world)
{
    if (m_settings.sync_meshes) {
        bl::BObject bobj(obj);
        bl::BMesh bmesh(data);
        bool is_editing = get_edit_mesh(bmesh.ptr()) != nullptr;

        // on edit mode, editing is applied to EditMesh and base Mesh is intact. so get data from EditMesh on edit mode.
        // todo: Blender 2.8 displays transparent final mesh on edit mode. extract data from it.
        if (is_editing) {
            doExtractEditMeshData(dst, obj, data);
        }
        else {
            doExtractNonEditMeshData(dst, obj, data);
        }

        if (!m_settings.bake_modifiers && !is_editing) {
            // mirror
            if (auto *mirror = (const MirrorModifierData*)find_modofier(obj, eModifierType_Mirror)) {
                if (mirror->flag & MOD_MIR_AXIS_X) dst.refine_settings.flags.mirror_x = 1;
                if (mirror->flag & MOD_MIR_AXIS_Y) dst.refine_settings.flags.mirror_y = 1;
                if (mirror->flag & MOD_MIR_AXIS_Z) dst.refine_settings.flags.mirror_z = 1;
                if (mirror->mirror_ob) {
                    dst.refine_settings.flags.mirror_basis = 1;
                    mu::float4x4 wm = bobj.matrix_world();
                    mu::float4x4 mm = bl::BObject(mirror->mirror_ob).matrix_world();
                    dst.refine_settings.mirror_basis = wm * mu::invert(mm);
                }
            }
        }
        if (m_settings.bake_transform) {
            dst.refine_settings.local2world = world;
            dst.refine_settings.flags.local2world = 1;
        }
    }
    else {
        if (!m_settings.bake_modifiers && m_settings.sync_blendshapes) {
            doExtractBlendshapeWeights(dst, obj, data);
        }
    }

    if (dst.normals.empty())
        dst.refine_settings.flags.gen_normals = 1;
    if (dst.tangents.empty())
        dst.refine_settings.flags.gen_tangents = 1;
    dst.refine_settings.flags.flip_faces = 1;
    dst.refine_settings.flags.make_double_sided = m_settings.make_double_sided;
}

void msblenContext::doExtractBlendshapeWeights(ms::Mesh& dst, Object *obj, Mesh *data)
{
    auto& mesh = *data;
    if (!m_settings.bake_modifiers) {
        // blend shapes
        if (m_settings.sync_blendshapes && mesh.key) {
            RawVector<mu::float3> basis;
            int bi = 0;
            each_key(&mesh, [&](const KeyBlock *kb) {
                if (bi == 0) { // Basis
                }
                else {
                    auto bsd = dst.addBlendShape(kb->name);
                    bsd->weight = kb->curval * 100.0f;
                }
                ++bi;
            });
        }
    }
}

void msblenContext::doExtractNonEditMeshData(ms::Mesh& dst, Object *obj, Mesh *data)
{
    bl::BObject bobj(obj);
    bl::BMesh bmesh(data);
    auto& mesh = *data;

    auto indices = bmesh.indices();
    auto polygons = bmesh.polygons();
    auto vertices = bmesh.vertices();

    size_t num_indices = indices.size();
    size_t num_polygons = polygons.size();
    size_t num_vertices = vertices.size();

    std::vector<int> mid_table(mesh.totcol);
    for (int mi = 0; mi < mesh.totcol; ++mi)
        mid_table[mi] = getMaterialID(mesh.mat[mi]);
    if (mid_table.empty())
        mid_table.push_back(ms::InvalidID);

    // vertices
    dst.points.resize_discard(num_vertices);
    for (size_t vi = 0; vi < num_vertices; ++vi) {
        dst.points[vi] = (mu::float3&)vertices[vi].co;
    }

    // faces
    dst.indices.reserve(num_indices);
    dst.counts.resize_discard(num_polygons);
    dst.material_ids.resize_discard(num_polygons);
    {
        int ii = 0;
        for (size_t pi = 0; pi < num_polygons; ++pi) {
            auto& polygon = polygons[pi];
            int material_index = polygon.mat_nr;
            int count = polygon.totloop;
            dst.counts[pi] = count;
            dst.material_ids[pi] = mid_table[material_index];
            dst.indices.resize(dst.indices.size() + count);

            auto *idx = &indices[polygon.loopstart];
            for (int li = 0; li < count; ++li) {
                dst.indices[ii++] = idx[li].v;
            }
        }
    }

    // normals
    if (m_settings.sync_normals) {
#if 0
        // per-vertex
        dst.normals.resize_discard(num_vertices);
        for (size_t vi = 0; vi < num_vertices; ++vi) {
            dst.normals[vi] = to_float3(vertices[vi].no);
        }
#endif
        // per-index
        auto normals = bmesh.normals();
        if (!normals.empty()) {
            dst.normals.resize_discard(num_indices);
            for (size_t ii = 0; ii < num_indices; ++ii)
                dst.normals[ii] = normals[ii];
        }
    }


    // uv
    if (m_settings.sync_uvs) {
        auto loop_uv = bmesh.uv();
        if (!loop_uv.empty()) {
            dst.uv0.resize_discard(num_indices);
            for (size_t ii = 0; ii < num_indices; ++ii)
                dst.uv0[ii] = (mu::float2&)loop_uv[ii].uv;
        }
    }

    // colors
    if (m_settings.sync_colors) {
        auto colors = bmesh.colors();
        if (!colors.empty()) {
            dst.colors.resize_discard(num_indices);
            for (size_t ii = 0; ii < num_indices; ++ii)
                dst.colors[ii] = to_float4(colors[ii]);
        }
    }

    if (!m_settings.bake_modifiers) {
        // bones

        auto extract_bindpose = [](auto *bone) {
            auto mat_bone = (mu::float4x4&)bone->arm_mat * g_arm_to_world;
            // armature-space to world-space
            return mu::invert(mu::swap_yz(mu::flip_z(mat_bone)));
        };

        if (m_settings.sync_bones) {
            auto *arm_mod = (const ArmatureModifierData*)find_modofier(obj, eModifierType_Armature);
            if (arm_mod) {
                // request bake TRS
                dst.refine_settings.flags.local2world = 1;
                dst.refine_settings.local2world = ms::transform(dst.position, invert(dst.rotation), dst.scale);

                auto *arm_obj = arm_mod->object;
                int group_index = 0;
                each_deform_group(obj, [&](const bDeformGroup *g) {
                    bool found = false;
                    auto bone = find_bone(arm_obj, g->name);
                    if (bone) {
                        auto trans = findBone(arm_obj, bone);
                        if (trans) {
                            found = true;
                            auto b = dst.addBone(trans->path);
                            b->bindpose = extract_bindpose(bone);
                            b->weights.resize_zeroclear(num_vertices);

                            for (int vi = 0; vi < num_vertices; ++vi) {
                                int num_weights = mesh.dvert[vi].totweight;
                                auto& dvert = mesh.dvert[vi];
                                for (int wi = 0; wi < num_weights; ++wi) {
                                    if (dvert.dw[wi].def_nr == group_index) {
                                        b->weights[vi] = dvert.dw[wi].weight;
                                    }
                                }
                            }
                        }
                    }
                    if (!found) {
                        mscTrace("bone not found %s\n", g->name);
                    }
                    ++group_index;
                });
            }
        }

        // blend shapes
        if (m_settings.sync_blendshapes && mesh.key) {
            RawVector<mu::float3> basis;
            int bi = 0;
            each_key(&mesh, [&](const KeyBlock *kb) {
                if (bi == 0) { // Basis
                    basis.resize_discard(kb->totelem);
                    memcpy(basis.data(), kb->data, basis.size() * sizeof(mu::float3));
                }
                else {
                    auto bsd = dst.addBlendShape(kb->name);
                    bsd->weight = kb->curval * 100.0f;

                    bsd->frames.push_back(ms::BlendShapeFrameData::create());
                    auto& frame = *bsd->frames.back();
                    frame.weight = 100.0f;
                    frame.points.resize_discard(kb->totelem);
                    memcpy(frame.points.data(), kb->data, basis.size() * sizeof(mu::float3));

                    size_t len = frame.points.size();
                    for (size_t i = 0; i < len; ++i) {
                        frame.points[i] -= basis[i];
                    }
                }
                ++bi;
            });
        }
    }

#if 0
    // lines
    // (blender doesn't include lines & points in polygons - MPoly::totloop is always >= 3)
    {
        auto edges = bmesh.edges();

        std::vector<bool> point_shared(num_vertices);
        for (size_t pi = 0; pi < num_polygons; ++pi) {
            auto& polygon = polygons[pi];
            int count = polygon.totloop;
            auto *idx = &indices[polygon.loopstart];
            for (int li = 0; li < count; ++li) {
                point_shared[idx[li].v] = true;
            }
        }

        size_t lines_begin = dst.indices.size();
        size_t num_lines = 0;
        for (auto edge : edges) {
            if (!point_shared[edge.v1] || !point_shared[edge.v2]) {
                ++num_lines;
                dst.counts.push_back(2);
                dst.indices.push_back(edge.v1);
                dst.indices.push_back(edge.v2);
            }
        }

        if (num_lines > 0) {
            num_indices = dst.indices.size();

            if (!dst.normals.empty() && m_settings.sync_normals == msbNormalSyncMode::PerIndex) {
                dst.normals.resize(num_indices, mu::float3::zero());
            }
            if (!dst.uv0.empty()) {
                dst.uv0.resize(num_indices, mu::float2::zero());
            }
            if (!dst.colors.empty()) {
                auto colors = bmesh.colors();
                dst.colors.resize(num_indices, mu::float4::one());
                for (size_t ii = lines_begin; ii < num_indices; ++ii) {
                    int vi = dst.indices[ii];
                    dst.colors[ii] = to_float4(colors[vi]);
                }
            }
        }
    }
#endif
}

void msblenContext::doExtractEditMeshData(ms::Mesh& dst, Object *obj, Mesh *data)
{
    bl::BObject bobj(obj);
    bl::BMesh bmesh(data);
    bl::BEditMesh emesh(get_edit_mesh(bmesh.ptr()));
    auto& mesh = *data;

    auto polygons = emesh.polygons();
    auto triangles = emesh.triangles();
    auto vertices = emesh.vertices();

    size_t num_triangles = triangles.size();
    size_t num_vertices = vertices.size();
    size_t num_indices = triangles.size() * 3;

    std::vector<int> mid_table(mesh.totcol);
    for (int mi = 0; mi < mesh.totcol; ++mi)
        mid_table[mi] = getMaterialID(mesh.mat[mi]);
    if (mid_table.empty())
        mid_table.push_back(-1);

    // vertices
    dst.points.resize_discard(num_vertices);
    for (size_t vi = 0; vi < num_vertices; ++vi) {
        dst.points[vi] = (mu::float3&)vertices[vi]->co;
    }

    // faces
    {
        dst.indices.resize(num_indices);
        dst.counts.resize_discard(num_triangles);
        dst.material_ids.resize_discard(num_triangles);

        size_t ii = 0;
        for (size_t ti = 0; ti < num_triangles; ++ti) {
            auto& triangle = triangles[ti];

            int material_index = 0;
            int polygon_index = triangle[0]->f->head.index;
            if (polygon_index < polygons.size())
                material_index = mid_table[polygons[polygon_index]->mat_nr];
            dst.material_ids[ti] = material_index;

            dst.counts[ti] = 3;
            for (auto *idx : triangle)
                dst.indices[ii++] = idx->v->head.index;
        }
    }

    // normals
    if (m_settings.sync_normals) {
#if 0
        // per-vertex
        dst.normals.resize_discard(num_vertices);
        for (size_t vi = 0; vi < num_vertices; ++vi)
            dst.normals[vi] = to_float3(vertices[vi]->no);
#endif
        // per-index
        dst.normals.resize_discard(num_indices);
        size_t ii = 0;
        for (size_t ti = 0; ti < num_triangles; ++ti) {
            auto& triangle = triangles[ti];
            for (auto *idx : triangle)
                dst.normals[ii++] = -bl::BM_loop_calc_face_normal(*idx);
        }
    }

    // uv
    if (m_settings.sync_uvs) {
        int offset = emesh.uv_data_offset();
        if (offset != -1) {
            dst.uv0.resize_discard(num_indices);
            size_t ii = 0;
            for (size_t ti = 0; ti < num_triangles; ++ti) {
                auto& triangle = triangles[ti];
                for (auto *idx : triangle)
                    dst.uv0[ii++] = *(mu::float2*)((char*)idx->head.data + offset);
            }
        }
    }
}

ms::TransformPtr msblenContext::findBone(Object *armature, Bone *bone)
{
    auto it = m_bones.find(bone);
    return it != m_bones.end() ? it->second : nullptr;
}

msblenContext::ObjectRecord& msblenContext::touchRecord(Object *obj, const std::string& base_path, bool children)
{
    auto& rec = m_obj_records[obj];
    if (rec.touched && base_path.empty())
        return rec; // already touched

    rec.touched = true;

    auto local_path = get_path(obj);
    if (local_path != rec.path) {
        rec.renamed = true;
        rec.path = local_path;
    }
    auto path = base_path + local_path;
    m_entity_manager.touch(path);

    // trace bones
    if (is_armature(obj)) {
        auto poses = bl::list_range((bPoseChannel*)obj->pose->chanbase.first);
        for (auto pose : poses) {
            m_obj_records[pose->bone].touched = true;
            m_entity_manager.touch(base_path + get_path(obj, pose->bone));
        }
    }

    // care children
    if (children) {
        each_child(obj, [&](Object *child) {
            touchRecord(child, base_path, true);
        });
    }

    // trace dupli group
    if (auto group = get_instance_collection(obj)) {
        auto group_path = path + '/' + (group->id.name + 2);
        m_entity_manager.touch(group_path);

        auto gobjects = bl::list_range((CollectionObject*)group->gobject.first);
        for (auto go : gobjects)
            touchRecord(go->ob, group_path, true);
    }
    return rec;
}

void msblenContext::eraseStaleObjects()
{
    for (auto i = m_obj_records.begin(); i != m_obj_records.end(); /**/) {
        if (!i->second.touched)
            m_obj_records.erase(i++);
        else
            ++i;
    }
    m_entity_manager.eraseStaleEntities();
}


void msblenContext::exportAnimation(Object *obj, bool force, const std::string& base_path)
{
    if (!obj)
        return;

    auto path = base_path + get_path(obj);
    if (m_anim_records.find(path) != m_anim_records.end())
        return;

    auto& clip = m_animations.front();
    ms::TransformAnimationPtr dst;
    AnimationRecord::extractor_t extractor = nullptr;
    auto group = get_instance_collection(obj);

    auto add_animation = [this, &clip](const std::string& path, void *obj, ms::TransformAnimationPtr dst, AnimationRecord::extractor_t extractor) {
        dst->path = path;
        auto& rec = m_anim_records[path];
        rec.extractor = extractor;
        rec.obj = obj;
        rec.dst = dst;
        clip->addAnimation(dst);
    };

    switch (obj->type) {
    case OB_CAMERA:
    {
        // camera
        exportAnimation(obj->parent, true, base_path);
        add_animation(path, obj, ms::CameraAnimation::create(), &msblenContext::extractCameraAnimationData);
        break;
    }
    case OB_LAMP:
    {
        // lights
        exportAnimation(obj->parent, true, base_path);
        add_animation(path, obj, ms::LightAnimation::create(), &msblenContext::extractLightAnimationData);
        break;
    }
    case OB_MESH:
    {
        // meshes
        exportAnimation(obj->parent, true, base_path);
        add_animation(path, obj, ms::MeshAnimation::create(), &msblenContext::extractMeshAnimationData);
        break;
    }
    default:
    if (force || obj->type == OB_ARMATURE || group) {
        exportAnimation(obj->parent, true, base_path);
        add_animation(path, obj, ms::TransformAnimation::create(), &msblenContext::extractTransformAnimationData);

        if (obj->type == OB_ARMATURE && (!m_settings.bake_modifiers && m_settings.sync_bones)) {
            // bones
            auto poses = bl::list_range((bPoseChannel*)obj->pose->chanbase.first);
            for (auto pose : poses) {
                auto pose_path = base_path + get_path(obj, pose->bone);
                add_animation(pose_path, pose, ms::TransformAnimation::create(), &msblenContext::extractPoseAnimationData);
            }
        }
        break;
    }
    }

    // handle dupli group
    if (group) {
        auto group_path = base_path;
        group_path += '/';
        group_path += get_name(obj);
        group_path += '/';
        group_path += (group->id.name + 2);

        auto gobjects = bl::list_range((CollectionObject*)group->gobject.first);
        for (auto go : gobjects) {
            exportAnimation(go->ob, false, group_path);
        }
    }
}

void msblenContext::extractTransformAnimationData(ms::TransformAnimation& dst_, void *obj)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    mu::float3 pos;
    mu::quatf rot;
    mu::float3 scale;
    ms::VisibilityFlags vis;
    extractTransformData((Object*)obj, pos, rot, scale, vis);

    float t = m_anim_time;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
    dst.visible.push_back({ t, (int)vis.visible_in_render });
}

void msblenContext::extractPoseAnimationData(ms::TransformAnimation& dst_, void *obj)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    mu::float3 t;
    mu::quatf r;
    mu::float3 s;
    extractTransformData((bPoseChannel*)obj, t, r, s);

    float time = m_anim_time;
    dst.translation.push_back({ time, t });
    dst.rotation.push_back({ time, r });
    dst.scale.push_back({ time, s });
}

void msblenContext::extractCameraAnimationData(ms::TransformAnimation& dst_, void *obj)
{
    extractTransformAnimationData(dst_, obj);

    auto& dst = (ms::CameraAnimation&)dst_;

    bool ortho;
    float near_plane, far_plane, fov, focal_length;
    mu::float2 sensor_size, lens_shift;
    extractCameraData((Object*)obj, ortho, near_plane, far_plane, fov, focal_length, sensor_size, lens_shift);

    float t = m_anim_time;
    dst.near_plane.push_back({ t , near_plane });
    dst.far_plane.push_back({ t , far_plane });
    dst.fov.push_back({ t , fov });
    dst.focal_length.push_back({ t , focal_length });
    dst.sensor_size.push_back({ t , sensor_size });
    dst.lens_shift.push_back({ t , lens_shift });
}

void msblenContext::extractLightAnimationData(ms::TransformAnimation& dst_, void *obj)
{
    extractTransformAnimationData(dst_, obj);

    auto& dst = (ms::LightAnimation&)dst_;

    ms::Light::LightType ltype;
    ms::Light::ShadowType stype;
    mu::float4 color;
    float intensity, range, spot_angle;
    extractLightData((Object*)obj, ltype, stype, color, intensity, range, spot_angle);

    float t = m_anim_time;
    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });
    dst.range.push_back({ t, range });
    if (ltype == ms::Light::LightType::Spot) {
        dst.spot_angle.push_back({ t, spot_angle });
    }
}

void msblenContext::extractMeshAnimationData(ms::TransformAnimation & dst_, void * obj)
{
    extractTransformAnimationData(dst_, obj);

    auto& dst = (ms::MeshAnimation&)dst_;
    float t = m_anim_time;

    auto& mesh = *(Mesh*)((Object*)obj)->data;
    if (!get_edit_mesh(&mesh) && mesh.key) {
        // blendshape weight animation
        int bi = 0;
        each_key(&mesh, [&](const KeyBlock *kb) {
            if (bi == 0) { // Basis
            }
            else {
                dst.getBlendshapeCurve(kb->name).push_back({ t, kb->curval * 100.0f });
            }
            ++bi;
        });
    }
}


void msblenContext::logInfo(const char * format, ...)
{
    const int MaxBuf = 2048;
    char buf[MaxBuf];

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    puts(buf);
    va_end(args);
}

bool msblenContext::isServerAvailable()
{
    m_sender.client_settings = m_settings.client_settings;
    return m_sender.isServerAvaileble();
}

const std::string& msblenContext::getErrorMessage()
{
    return m_sender.getErrorMessage();
}

void msblenContext::wait()
{
    m_sender.wait();
}

void msblenContext::clear()
{
    m_material_ids.clear();
    m_texture_manager.clear();
    m_material_manager.clear();
    m_entity_manager.clear();
}

bool msblenContext::prepare()
{
    if (!bl::ready())
        return false;
    return true;
}

bool msblenContext::sendMaterials(bool dirty_all)
{
    if (!prepare() || m_sender.isExporting() || m_ignore_events)
        return false;

    m_settings.validate();
    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(dirty_all);
    exportMaterials();

    // send
    kickAsyncExport();
    return true;
}

bool msblenContext::sendObjects(ObjectScope scope, bool dirty_all)
{
    if (!prepare() || m_sender.isExporting() || m_ignore_events)
        return false;

    m_settings.validate();
    m_entity_manager.setAlwaysMarkDirty(dirty_all);
    m_material_manager.setAlwaysMarkDirty(dirty_all);
    m_texture_manager.setAlwaysMarkDirty(false); // false because too heavy

    if (m_settings.sync_meshes)
        exportMaterials();

    if (scope == ObjectScope::Updated) {
        auto bpy_data = bl::BData(bl::BContext::get().data());
        if (!bpy_data.objects_is_updated())
            return true; // nothing to send

        auto scene = bl::BScene(bl::BContext::get().scene());
        scene.each_objects([this](Object *obj) {
            auto bid = bl::BID(obj);
            if (bid.is_updated() || bid.is_updated_data())
                exportObject(obj, false);
            else
                touchRecord(obj); // this cannot be covered by getNodes()
        });
        eraseStaleObjects();
    }
    else {
        for(auto obj : getNodes(scope))
            exportObject(obj, true);
        eraseStaleObjects();
    }

    kickAsyncExport();
    return true;
}

bool msblenContext::sendAnimations(ObjectScope scope)
{
    if (!prepare() || m_sender.isExporting() || m_ignore_events)
        return false;

    m_settings.validate();
    m_ignore_events = true;

    auto scene = bl::BScene(bl::BContext::get().scene());
    const int frame_rate = scene.fps();
    const int frame_step = std::max(m_settings.frame_step, 1);

    m_animations.clear();
    m_animations.push_back(ms::AnimationClip::create()); // create default clip

    auto& clip = *m_animations.back();
    clip.frame_rate = (float)frame_rate;

    // list target objects
    for (auto obj : getNodes(scope))
        exportAnimation(obj, false);

    // advance frame and record animations
    {
        int frame_current = scene.frame_current();
        int frame_start = scene.frame_start();
        int frame_end = scene.frame_end();
        int interval = frame_step;
        int reserve_size = (frame_end - frame_start) / interval + 1;

        for (auto& kvp : m_anim_records) {
            kvp.second.dst->reserve(reserve_size);
        };
        for (int f = frame_start;;) {
            scene.frame_set(f);
            m_anim_time = float(f - frame_start) / frame_rate;

            mu::parallel_for_each(m_anim_records.begin(), m_anim_records.end(), [this](auto& kvp) {
                kvp.second(this);
            });

            if (f >= frame_end)
                break;
            else
                f = std::min(f + interval, frame_end);
        }
        m_anim_records.clear();
        scene.frame_set(frame_current);
    }

    m_ignore_events = false;

    // send
    if (!m_animations.empty()) {
        kickAsyncExport();
        return true;
    }
    return false;
}

bool msblenContext::exportCache(const CacheSettings& cache_settings)
{
    auto scene = bl::BScene(bl::BContext::get().scene());
    const int frame_rate = scene.fps();
    const int frame_step = std::max(cache_settings.frame_step, 1);

    auto settings_old = m_settings;
    m_settings.export_cache = true;
    m_settings.make_double_sided = cache_settings.make_double_sided;
    m_settings.curves_as_mesh = cache_settings.curves_as_mesh;
    m_settings.bake_modifiers = cache_settings.bake_modifiers;
    m_settings.bake_transform = cache_settings.bake_transform;
    m_settings.flatten_hierarchy = cache_settings.flatten_hierarchy;
    m_settings.validate();

    ms::OSceneCacheSettings oscs;
    oscs.sample_rate = (float)frame_rate;
    oscs.encoder_settings.zstd.compression_level = cache_settings.zstd_compression_level;
    oscs.flatten_hierarchy = cache_settings.flatten_hierarchy;
    oscs.strip_normals = cache_settings.strip_normals;
    oscs.strip_tangents = cache_settings.strip_tangents;

    if (!m_cache_writer.open(cache_settings.path.c_str(), oscs)) {
        m_settings = settings_old;
        return false;
    }

    m_material_manager.setAlwaysMarkDirty(true);
    m_entity_manager.setAlwaysMarkDirty(true);

    int scene_index = 0;
    auto material_range = cache_settings.material_frame_range;
    auto nodes = getNodes(cache_settings.object_scope);

    auto do_export = [&]() {
        if (scene_index == 0) {
            // exportMaterials() is needed to export material IDs in meshes
            exportMaterials();
            if (material_range == MaterialFrameRange::None)
                m_material_manager.clearDirtyFlags();
        }
        else {
            if (material_range == MaterialFrameRange::All)
                exportMaterials();
        }

        for (auto& n : nodes)
            exportObject(n, true);

        m_texture_manager.clearDirtyFlags();
        kickAsyncExport();
        ++scene_index;
    };

    if (cache_settings.frame_range == FrameRange::Current) {
        m_anim_time = 0.0f;
        do_export();
    }
    else {
        int frame_current = scene.frame_current();
        int frame_start, frame_end;
        // time range
        if (cache_settings.frame_range == FrameRange::Custom) {
            // custom frame range
            frame_start = cache_settings.frame_begin;
            frame_end = cache_settings.frame_end;
        }
        else {
            // all active frames
            frame_start = scene.frame_start();
            frame_end = scene.frame_end();
        }
        int interval = frame_step;

        // record
        for (int f = frame_start;;) {
            scene.frame_set(f);
            m_anim_time = float(f - frame_start) / frame_rate;

            do_export();

            if (f >= frame_end)
                break;
            else
                f = std::min(f + interval, frame_end);
        }
        scene.frame_set(frame_current);
    }

    m_settings = settings_old;
    m_cache_writer.close();
    return true;
}

void msblenContext::flushPendingList()
{
    if (!m_pending.empty() && !m_sender.isExporting()) {
        for (auto p : m_pending)
            exportObject(p, false);
        m_pending.clear();
        kickAsyncExport();
    }
}

void msblenContext::kickAsyncExport()
{
    for (auto& t : m_async_tasks)
        t.wait();
    m_async_tasks.clear();

    // clear baked meshes
#if BLENDER_VERSION < 280
    if (!m_tmp_meshes.empty()) {
        bl::BData bd(bl::BContext::get().data());
        for (auto *v : m_tmp_meshes)
            bd.remove(v);
        m_tmp_meshes.clear();
}
#else
    if (!m_meshes_to_clear.empty()) {
        for (auto *v : m_meshes_to_clear) {
            bl::BObject bobj(v);
            bobj.to_mesh_clear();
        }
        m_meshes_to_clear.clear();
    }
#endif

    for (auto& kvp : m_obj_records)
        kvp.second.clearState();
    m_bones.clear();

    using Exporter = ms::AsyncSceneExporter;
    Exporter *exporter = m_settings.export_cache ? (Exporter*)&m_cache_writer : (Exporter*)&m_sender;

    // kick async send
    exporter->on_prepare = [this, exporter]() {
        if (auto sender = dynamic_cast<ms::AsyncSceneSender*>(exporter)) {
            sender->client_settings = m_settings.client_settings;
        }
        else if (auto writer = dynamic_cast<ms::AsyncSceneCacheWriter*>(exporter)) {
            writer->time = m_anim_time;
        }

        auto& t = *exporter;
        t.scene_settings = m_settings.scene_settings;
        float scale_factor = 1.0f / m_settings.scene_settings.scale_factor;
        t.scene_settings.scale_factor = 1.0f;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();
        t.animations = m_animations;

        t.deleted_materials = m_material_manager.getDeleted();
        t.deleted_entities = m_entity_manager.getDeleted();

        if (scale_factor != 1.0f) {
            ms::ScaleConverter cv(scale_factor);
            for (auto& obj : t.transforms) { cv.convert(*obj); }
            for (auto& obj : t.geometries) { cv.convert(*obj); }
            for (auto& obj : t.animations) { cv.convert(*obj); }
        }
    };
    exporter->on_success = [this]() {
        m_material_ids.clearDirtyFlags();
        m_texture_manager.clearDirtyFlags();
        m_material_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
        m_animations.clear();
    };
    exporter->kick();
}
