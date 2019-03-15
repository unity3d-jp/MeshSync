#include "pch.h"
#include "MeshSyncClientBlender.h"
#include "msbContext.h"
#include "msbUtils.h"
using namespace mu;


msbContext::msbContext()
{
    m_settings.scene_settings.handedness = ms::Handedness::LeftZUp;
}

msbContext::~msbContext()
{
    m_sender.wait();
}

msbSettings& msbContext::getSettings() { return m_settings; }
const msbSettings& msbContext::getSettings() const { return m_settings; }


int msbContext::exportTexture(const std::string & path, ms::TextureType type)
{
    return m_texture_manager.addFile(path, type);
}

void msbContext::exportMaterials()
{
    int midx = 0;
    auto bpy_data = bl::BData(bl::BContext::get().data());
    for (auto *mat : bpy_data.materials()) {
        auto ret = ms::Material::create();
        ret->name = mat->id.name + 2;
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
        stdmat.setColor(float4{ color_src->r, color_src->g, color_src->b, 1.0f });

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

        m_material_manager.add(ret);
    }
    m_material_ids.eraseStaleRecords();
    m_material_manager.eraseStaleMaterials();
}


static void ExtractTransformData(Object *src, float3& t, quatf& r, float3& s, bool& vis)
{
    extract_local_TRS(src, t, r, s);
    vis = is_visible(src);
}
static void ExtractTransformData(Object *src, ms::Transform& dst)
{
    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible);
}

static void ExtractCameraData(Object *src, bool& ortho, float& near_plane, float& far_plane, float& fov)
{
    auto data = (Camera*)src->data;
    ortho = data->type == CAM_ORTHO;
#if BLENDER_VERSION < 280
    near_plane = data->clipsta;
    far_plane = data->clipend;
#else
    near_plane = data->clip_start;
    far_plane = data->clip_end;
#endif
    fov = bl::BCamera(data).fov_vertical() * mu::Rad2Deg;
}

static void ExtractLightData(Object *src, ms::Light::LightType& type, float4& color, float& intensity, float& range, float& spot_angle)
{
    auto data =
#if BLENDER_VERSION < 280
        (Lamp*)src->data;
#else
        (Light*)src->data;
#endif
    color = (float4&)data->r;
    intensity = data->energy;
    range = data->dist;

    switch (data->type) {
    case LA_SUN:
        type = ms::Light::LightType::Directional;
        break;
    case LA_SPOT:
        type = ms::Light::LightType::Spot;
        spot_angle = data->spotsize * mu::Rad2Deg;
        break;
    case LA_AREA:
        type = ms::Light::LightType::Area;
        break;
    default:
        type = ms::Light::LightType::Point;
        break;
    }
}


ms::TransformPtr msbContext::exportObject(Object *obj, bool force)
{
    ms::TransformPtr ret;
    if (!obj)
        return ret;

    auto& rec = touchRecord(obj);
    if (rec.exported)
        return ret; // already exported
    rec.exported = true;

    bool exported = false;
    auto on_export = [this, obj, &exported]() {
        exportObject(obj->parent, true);
        exported = true;
    };

    switch (obj->type) {
    case OB_ARMATURE:
    {
        if (m_settings.sync_bones) {
            on_export();
            ret = exportArmature(obj);
        }
        break;
    }
    case OB_MESH:
    {
        if (m_settings.sync_meshes || m_settings.sync_blendshapes) {
            on_export();
            ret = exportMesh(obj);
        }
        break;
    }
    case OB_FONT:  //
    case OB_CURVE: //
    case OB_SURF:  //
    case OB_MBALL: // these can be converted to mesh
    {
        if (m_settings.sync_meshes && m_settings.convert_to_mesh) {
            on_export();
            ret = exportMesh(obj);
        }
        break;
    }
    case OB_CAMERA:
    {
        if (m_settings.sync_cameras) {
            on_export();
            ret = exportCamera(obj);
        }
        break;
    }
    case OB_LAMP:
    {
        if (m_settings.sync_lights) {
            on_export();
            ret = exportLight(obj);
        }
        break;
    }
    default:
    {
        if (get_instance_collection(obj) || force) {
            on_export();
            ret = exportTransform(obj);
        }
        break;
    }
    }

    if (ret)
        exportDupliGroup(obj, ret->path);
    if (!exported)
        m_entity_manager.erase(rec.path);
    return ret;
}

ms::TransformPtr msbContext::exportTransform(Object *src)
{
    auto ret = ms::Transform::create();
    auto& dst = *ret;
    dst.path = get_path(src);
    ExtractTransformData(src, dst);
    m_entity_manager.add(ret);
    return ret;
}

ms::TransformPtr msbContext::exportPose(Object *armature, bPoseChannel *src)
{
    auto ret = ms::Transform::create();
    auto& dst = *ret;
    dst.path = get_path(armature, src->bone);
    extract_local_TRS(src, dst.position, dst.rotation, dst.scale);
    m_entity_manager.add(ret);
    return ret;
}

ms::TransformPtr msbContext::exportArmature(Object *src)
{
    auto ret = ms::Transform::create();
    auto& dst = *ret;
    dst.path = get_path(src);
    ExtractTransformData(src, dst);
    m_entity_manager.add(ret);

    for (auto pose : bl::list_range((bPoseChannel*)src->pose->chanbase.first)) {
        auto bone = pose->bone;
        auto& dst = m_bones[bone];
        dst = exportPose(src, pose);
    }
    return ret;
}

ms::TransformPtr msbContext::exportReference(Object *src, const std::string& base_path)
{
    auto local_path = get_path(src);
    auto path = base_path + local_path;

    auto dst = ms::Transform::create();
    dst->path = path;
    dst->reference = local_path;
    ExtractTransformData(src, *dst);
    exportDupliGroup(src, path);
    m_entity_manager.add(dst);

    each_child(src, [this, &path](Object *child) {
        exportReference(child, path);
    });
    return dst;
}

ms::TransformPtr msbContext::exportDupliGroup(Object *src, const std::string& base_path)
{
    auto group = get_instance_collection(src);
    if (!group)
        return nullptr;

    auto local_path = std::string("/") + (group->id.name + 2);
    auto path = base_path + local_path;

    auto dst = ms::Transform::create();
    dst->path = path;
    dst->position = -swap_yz(get_instance_offset(group));
    dst->visible_hierarchy = is_visible(src);
    m_entity_manager.add(dst);

    auto gobjects = bl::list_range((CollectionObject*)group->gobject.first);
    for (auto go : gobjects) {
        auto obj = go->ob;
        if (auto t = exportObject(obj, false)) {
            t->visible = obj->id.lib == nullptr;
        }
        exportReference(obj, path);
    }
    return dst;
}

ms::CameraPtr msbContext::exportCamera(Object *src)
{
    auto ret = ms::Camera::create();
    auto& dst = *ret;
    dst.path = get_path(src);
    ExtractTransformData(src, dst);
    dst.rotation *= rotateX(90.0f * Deg2Rad);

    ExtractCameraData(src, dst.is_ortho, dst.near_plane, dst.far_plane, dst.fov);
    m_entity_manager.add(ret);
    return ret;
}

ms::LightPtr msbContext::exportLight(Object *src)
{
    auto ret = ms::Light::create();
    auto& dst = *ret;
    dst.path = get_path(src);
    ExtractTransformData(src, dst);
    dst.rotation *= rotateX(90.0f * Deg2Rad);

    ExtractLightData(src, dst.light_type, dst.color, dst.intensity, dst.range, dst.spot_angle);
    m_entity_manager.add(ret);
    return ret;
}

ms::MeshPtr msbContext::exportMesh(Object *src)
{
    // ignore particles
    if (//find_modofier(src, eModifierType_ParticleSystem) ||
        find_modofier(src, eModifierType_ParticleInstance) )
        return nullptr;

    bl::BObject bobj(src);
    Mesh *data = nullptr;
    if (src->type == OB_MESH)
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
    ExtractTransformData(src, dst);

    if (m_settings.sync_meshes) {
        bool need_convert = 
            (!is_editing && m_settings.bake_modifiers) ||
            (src->type != OB_MESH);

        if (need_convert) {
            Mesh *tmp =
#if BLENDER_VERSION < 280
                bobj.to_mesh(bl::BContext::get().scene());
#else
                bobj.to_mesh(bl::BContext::get().depsgraph());
#endif
            if (tmp) {
                data = tmp;
                m_tmp_meshes.push_back(tmp); // need to delete baked meshes later
            }
        }

        // calculate per index normals
        // note: when bake_modifiers is enabled, it is done for baked meshes
        if (m_settings.sync_normals && m_settings.calc_per_index_normals) {
            // calc_normals_split() seems can't be multi-threaded. it will cause unpredictable crash...
            // todo: calculate normals by myself to be multi-threaded
            bl::BMesh(data).calc_normals_split();
        }
    }

    auto task = [this, ret, src, data]() {
        auto& dst = *ret;
        doExtractMeshData(dst, src, data);
        m_entity_manager.add(ret);
    };

    if(m_settings.multithreaded)
        m_async_tasks.push_back(std::async(std::launch::async, task));
    else
        task();
    return ret;
}

void msbContext::doExtractMeshData(ms::Mesh& dst, Object *obj, Mesh *data)
{
    if (m_settings.sync_meshes) {
        bl::BObject bobj(obj);
        bl::BMesh bmesh(data);
        bool is_editing = get_edit_mesh(bmesh.ptr()) != nullptr;

        dst.refine_settings.flags.swap_faces = true;
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
                if (mirror->flag & MOD_MIR_AXIS_Y) dst.refine_settings.flags.mirror_z = 1;
                if (mirror->flag & MOD_MIR_AXIS_Z) dst.refine_settings.flags.mirror_y = 1;
                if (mirror->mirror_ob) {
                    dst.refine_settings.flags.mirror_basis = 1;
                    float4x4 wm = bobj.matrix_world();
                    float4x4 mm = bl::BObject(mirror->mirror_ob).matrix_world();
                    dst.refine_settings.mirror_basis = mu::swap_yz(wm * mu::invert(mm));
                }
            }
        }
        dst.convertHandedness_Mesh(false, true);
        dst.convertHandedness_BlendShapes(false, true);
    }
    else {
        if (!m_settings.bake_modifiers && m_settings.sync_blendshapes) {
            doExtractBlendshapeWeights(dst, obj, data);
        }
    }

    dst.setupFlags();
    dst.flags.apply_trs = true;
    dst.flags.has_refine_settings = true;
    if (!dst.flags.has_normals)
        dst.refine_settings.flags.gen_normals = true;
    if (!dst.flags.has_tangents)
        dst.refine_settings.flags.gen_tangents = true;
    dst.refine_settings.flags.make_double_sided = m_settings.make_double_sided;
}

void msbContext::doExtractBlendshapeWeights(ms::Mesh& dst, Object *obj, Mesh *data)
{
    auto& mesh = *data;
    if (!m_settings.bake_modifiers) {
        // blend shapes
        if (m_settings.sync_blendshapes && mesh.key) {
            RawVector<float3> basis;
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

void msbContext::doExtractNonEditMeshData(ms::Mesh& dst, Object *obj, Mesh *data)
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
        mid_table[mi] = m_material_ids.getID(mesh.mat[mi]);
    if (mid_table.empty())
        mid_table.push_back(ms::InvalidID);

    // vertices
    dst.points.resize_discard(num_vertices);
    for (size_t vi = 0; vi < num_vertices; ++vi) {
        dst.points[vi] = (float3&)vertices[vi].co;
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
                dst.uv0[ii] = (float2&)loop_uv[ii].uv;
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
        if (m_settings.sync_bones) {
            auto *arm_mod = (const ArmatureModifierData*)find_modofier(obj, eModifierType_Armature);
            if (arm_mod) {
                // request bake TRS
                dst.refine_settings.flags.apply_local2world = 1;
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
            RawVector<float3> basis;
            int bi = 0;
            each_key(&mesh, [&](const KeyBlock *kb) {
                if (bi == 0) { // Basis
                    basis.resize_discard(kb->totelem);
                    memcpy(basis.data(), kb->data, basis.size() * sizeof(float3));
                }
                else {
                    auto bsd = dst.addBlendShape(kb->name);
                    bsd->weight = kb->curval * 100.0f;

                    bsd->frames.push_back(ms::BlendShapeFrameData::create());
                    auto& frame = *bsd->frames.back();
                    frame.weight = 100.0f;
                    frame.points.resize_discard(kb->totelem);
                    memcpy(frame.points.data(), kb->data, basis.size() * sizeof(float3));

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
                dst.normals.resize(num_indices, float3::zero());
            }
            if (!dst.uv0.empty()) {
                dst.uv0.resize(num_indices, float2::zero());
            }
            if (!dst.colors.empty()) {
                auto colors = bmesh.colors();
                dst.colors.resize(num_indices, float4::one());
                for (size_t ii = lines_begin; ii < num_indices; ++ii) {
                    int vi = dst.indices[ii];
                    dst.colors[ii] = to_float4(colors[vi]);
                }
            }
        }
    }
#endif
}

void msbContext::doExtractEditMeshData(ms::Mesh& dst, Object *obj, Mesh *data)
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
        mid_table[mi] = m_material_ids.getID(mesh.mat[mi]);
    if (mid_table.empty())
        mid_table.push_back(-1);

    // vertices
    dst.points.resize_discard(num_vertices);
    for (size_t vi = 0; vi < num_vertices; ++vi) {
        dst.points[vi] = (float3&)vertices[vi]->co;
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
                    dst.uv0[ii++] = *(float2*)((char*)idx->head.data + offset);
            }
        }
    }
}

ms::TransformPtr msbContext::findBone(Object *armature, Bone *bone)
{
    auto it = m_bones.find(bone);
    return it != m_bones.end() ? it->second : nullptr;
}

msbContext::ObjectRecord& msbContext::touchRecord(Object *obj, const std::string& base_path)
{
    auto& rec = m_obj_records[obj];
    if (rec.touched)
        return rec; // already touched

    rec.touched = true;

    auto path = base_path + get_path(obj);
    if (path != rec.path) {
        rec.renamed = true;
        rec.path = path;
    }
    m_entity_manager.touch(path);

    // trace bones
    if (obj->type == OB_ARMATURE) {
        auto poses = bl::list_range((bPoseChannel*)obj->pose->chanbase.first);
        for (auto pose : poses) {
            m_obj_records[pose->bone].touched = true;
            m_entity_manager.touch(base_path + get_path(obj, pose->bone));
        }
    }

    // trace dupli group
    if (auto group = get_instance_collection(obj)) {
        auto local_path = std::string("/") + (group->id.name + 2);
        auto group_path = base_path + local_path;
        m_entity_manager.touch(group_path);

        auto gobjects = bl::list_range((CollectionObject*)group->gobject.first);
        for (auto go : gobjects) {
            touchRecord(obj, group_path);
        }
    }
    return rec;
}

void msbContext::eraseStaleObjects()
{
    for (auto i = m_obj_records.begin(); i != m_obj_records.end(); /**/) {
        if (!i->second.touched)
            m_obj_records.erase(i++);
        else
            ++i;
    }
    m_entity_manager.eraseStaleEntities();
}


void msbContext::sendAnimations(SendScope scope)
{
    // wait previous request to complete
    m_sender.wait();

    m_sending_animations = true;

    auto scene = bl::BScene(bl::BContext::get().scene());
    m_animations.clear();
    m_animations.push_back(ms::AnimationClip::create()); // create default clip

    // list target objects
    if (scope == SendScope::Selected) {
        // todo
    }
    else {
        // all
        scene.each_objects([this](Object *obj) {
            exportAnimation(obj, false);
        });
    }

    // advance frame and record animations
    {
        int frame_current = scene.frame_current();
        int frame_start = scene.frame_start();
        int frame_end = scene.frame_end();
        int interval = std::max(m_settings.animation_frame_interval, 1);
        float frame_to_seconds = 1.0f / scene.fps();
        int reserve_size = (frame_end - frame_start) / interval + 1;

        for(auto& kvp : m_anim_records) {
            kvp.second.dst->reserve(reserve_size);
        };
        for (int f = frame_start;;) {
            scene.frame_set(f);

            m_anim_time = (f - frame_start) * frame_to_seconds * m_settings.animation_timescale;
            mu::parallel_for_each(m_anim_records.begin(), m_anim_records.end(), [this](AnimationRecords::value_type& kvp) {
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

    if (m_settings.keyframe_reduction) {
        // keyframe reduction
        for (auto& clip : m_animations) {
            clip->reduction();
        }
        // erase empty clip
        m_animations.erase(
            std::remove_if(m_animations.begin(), m_animations.end(), [](ms::AnimationClipPtr& p) { return p->empty(); }),
            m_animations.end());
    }

    // send
    if (!m_animations.empty()) {
        kickAsyncSend();
    }
    m_sending_animations = false;
}

void msbContext::exportAnimation(Object *obj, bool force, const std::string base_path)
{
    if (!obj)
        return;

    auto path = base_path + get_path(obj);
    if (m_anim_records.find(path) != m_anim_records.end())
        return;

    auto& clip = m_animations.front();
    ms::AnimationPtr dst;
    AnimationRecord::extractor_t extractor = nullptr;
    auto group = get_instance_collection(obj);

    auto add_animation = [this, &clip](const std::string& path, void *obj, ms::AnimationPtr dst, AnimationRecord::extractor_t extractor) {
        dst->path = path;
        auto& rec = m_anim_records[path];
        rec.extractor = extractor;
        rec.obj = obj;
        rec.dst = dst.get();
        clip->animations.push_back(dst);
    };

    switch (obj->type) {
    case OB_CAMERA:
    {
        // camera
        exportAnimation(obj->parent, true, base_path);
        add_animation(path, obj, ms::CameraAnimation::create(), &msbContext::extractCameraAnimationData);
        break;
    }
    case OB_LAMP:
    {
        // lights
        exportAnimation(obj->parent, true, base_path);
        add_animation(path, obj, ms::LightAnimation::create(), &msbContext::extractLightAnimationData);
        break;
    }
    case OB_MESH:
    {
        // meshes
        exportAnimation(obj->parent, true, base_path);
        add_animation(path, obj, ms::MeshAnimation::create(), &msbContext::extractMeshAnimationData);
        break;
    }
    default:
    if (force || obj->type == OB_ARMATURE || group) {
        exportAnimation(obj->parent, true, base_path);
        add_animation(path, obj, ms::TransformAnimation::create(), &msbContext::extractTransformAnimationData);

        if (obj->type == OB_ARMATURE) {
            // bones
            auto poses = bl::list_range((bPoseChannel*)obj->pose->chanbase.first);
            for (auto pose : poses) {
                auto pose_path = base_path + get_path(obj, pose->bone);
                add_animation(pose_path, pose, ms::TransformAnimation::create(), &msbContext::extractPoseAnimationData);
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

void msbContext::extractTransformAnimationData(ms::Animation& dst_, void *obj)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    float3 pos;
    quatf rot;
    float3 scale;
    bool vis;
    extract_local_TRS((Object*)obj, pos, rot, scale);
    vis = is_visible((Object*)obj);

    float t = m_anim_time;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
    //dst.visible.push_back({ t, vis });
}

void msbContext::extractPoseAnimationData(ms::Animation& dst_, void * obj)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    float3 pos;
    quatf rot;
    float3 scale;
    extract_local_TRS((bPoseChannel*)obj, pos, rot, scale);

    float t = m_anim_time;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
}

void msbContext::extractCameraAnimationData(ms::Animation& dst_, void *obj)
{
    extractTransformAnimationData(dst_, obj);

    auto& dst = (ms::CameraAnimation&)dst_;
    {
        auto& last = dst.rotation.back();
        last.value *= rotateX(90.0f * Deg2Rad);
    }

    bool ortho;
    float near_plane, far_plane, fov;
    ExtractCameraData((Object*)obj, ortho, near_plane, far_plane, fov);

    float t = m_anim_time;
    dst.near_plane.push_back({ t , near_plane });
    dst.far_plane.push_back({ t , far_plane });
    dst.fov.push_back({ t , fov });
}

void msbContext::extractLightAnimationData(ms::Animation& dst_, void *obj)
{
    extractTransformAnimationData(dst_, obj);

    auto& dst = (ms::LightAnimation&)dst_;
    {
        auto& last = dst.rotation.back();
        last.value *= rotateX(90.0f * Deg2Rad);
    }

    ms::Light::LightType type;
    float4 color;
    float intensity, range, spot_angle;
    ExtractLightData((Object*)obj, type, color, intensity, range, spot_angle);

    float t = m_anim_time;
    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });
    dst.range.push_back({ t, range });
    if (type == ms::Light::LightType::Spot) {
        dst.spot_angle.push_back({ t, spot_angle });
    }
}

void msbContext::extractMeshAnimationData(ms::Animation & dst_, void * obj)
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
                auto bsa = dst.findOrCreateBlendshapeAnimation(kb->name);
                bsa->weight.push_back({ t, kb->curval * 100.0f });
            }
            ++bi;
        });
    }
}



void msbContext::clear()
{
    m_material_ids.clear();
    m_texture_manager.clear();
    m_material_manager.clear();
    m_entity_manager.clear();
}

bool msbContext::prepare()
{
    if (!bl::ready() || m_sender.isSending())
        return false;

    return true;
}

void msbContext::sendScene(SendScope scope, bool force_all)
{
    if (m_sending_animations || !prepare())
        return;

    if (force_all) {
        m_material_manager.makeDirtyAll();
        m_entity_manager.makeDirtyAll();
    }

    if (m_settings.sync_meshes)
        exportMaterials();

    if (scope == SendScope::Updated) {
        auto bpy_data = bl::BData(bl::BContext::get().data());
        if (!bpy_data.objects_is_updated())
            return; // nothing to send

        auto scene = bl::BScene(bl::BContext::get().scene());
        scene.each_objects([this](Object *obj) {
            auto bid = bl::BID(obj);
            if (bid.is_updated() || bid.is_updated_data())
                exportObject(obj, false);
            else
                touchRecord(obj);
        });
    }
    if (scope == SendScope::Selected) {
        // todo
    }
    else {
        // all
        auto scene = bl::BScene(bl::BContext::get().scene());
        scene.each_objects([this](Object *obj) {
            exportObject(obj, false);
        });
    }

    eraseStaleObjects();
    kickAsyncSend();
}

void msbContext::flushPendingList()
{
    if (!m_pending.empty() && !m_sender.isSending()) {
        for (auto p : m_pending)
            exportObject(p, false);
        m_pending.clear();
        kickAsyncSend();
    }
}

void msbContext::kickAsyncSend()
{
    for (auto& t : m_async_tasks)
        t.wait();
    m_async_tasks.clear();

    // clear baked meshes
    if (!m_tmp_meshes.empty()) {
        bl::BData bd(bl::BContext::get().data());
        for (auto *v : m_tmp_meshes)
            bd.remove(v);
        m_tmp_meshes.clear();
    }

    for (auto& kvp : m_obj_records)
        kvp.second.clearState();
    m_bones.clear();

    // kick async send
    m_sender.on_prepare = [this]() {
        auto& t = m_sender;
        t.client_settings = m_settings.client_settings;
        t.scene_settings = m_settings.scene_settings;
        float scale_factor = 1.0f / m_settings.scene_settings.scale_factor;
        t.scene_settings.scale_factor = 1.0f;
        t.scene_settings.handedness = ms::Handedness::Left;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();
        t.animations = m_animations;

        t.deleted_materials = m_material_manager.getDeleted();
        t.deleted_entities = m_entity_manager.getDeleted();

        if (scale_factor != 1.0f) {
            for (auto& obj : t.transforms) { obj->applyScaleFactor(scale_factor); }
            for (auto& obj : t.geometries) { obj->applyScaleFactor(scale_factor); }
            for (auto& obj : t.animations) { obj->applyScaleFactor(scale_factor); }
        }
    };
    m_sender.on_success = [this]() {
        m_material_ids.clearDirtyFlags();
        m_texture_manager.clearDirtyFlags();
        m_material_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
        m_animations.clear();
    };
    m_sender.kick();
}
