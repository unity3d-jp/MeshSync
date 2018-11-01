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

ms::MaterialPtr msbContext::addMaterial(Material * mat)
{
    auto ret = ms::Material::create();
    ret->name = mat->id.name + 2;
    ret->id = (int)m_materials.size();
    {
        bl::BMaterial bm(mat);
        auto color_src = mat;
        if (bm.use_nodes()) {
            bl::BMaterial node(bm.active_node_material());
            if (node.ptr()) {
                color_src = node.ptr();
            }
        }
        ret->setColor(float4{ color_src->r, color_src->g, color_src->b, 1.0f });

        if (m_settings.sync_textures) {
            auto export_texture = [this](MTex *mtex, ms::TextureType type) -> int {
                if (!mtex || !mtex->tex || !mtex->tex->ima)
                    return -1;
                return exportTexture(bl::abspath(mtex->tex->ima->name), type);
            };
#if BLENDER_VERSION < 280
            ret->setColorMap(export_texture(mat->mtex[0], ms::TextureType::Default));
#endif
        }
    }
    m_materials.push_back(ret);
    return ret;
}

int msbContext::getMaterialID(const Material *mat)
{
    if (mat == nullptr)
        return 0;

    int i = 0;
    for (auto& m : m_materials) {
        if (m->name == mat->id.name + 2)
            return i;
        ++i;
    }
    return 0;
}

void msbContext::exportMaterials()
{
    m_materials.clear();
    auto bpy_data = bl::BData(bl::BContext::get().data());;
    for (auto *mat : bpy_data.materials()) {
        addMaterial(mat);
    }
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
    near_plane = data->clipsta;
    far_plane = data->clipend;
    fov = bl::BCamera(data).fov_vertical() * mu::Rad2Deg;
}

static void ExtractLightData(Object *src, ms::Light::LightType& type, float4& color, float& intensity, float& range, float& spot_angle)
{
    auto data = (Lamp*)src->data;
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


void msbContext::addDeleted(const ms::Identifier& v)
{
    m_deleted.push_back(v);
    m_entity_manager.erase(v);
}

ms::TransformPtr msbContext::exportObject(Object *obj, bool force)
{
    ms::TransformPtr ret;
    if (!obj)
        return ret;

    auto& rec = touchRecord(obj);
    if (rec.exported)
        return ret; // already exported

    switch (obj->type) {
    case OB_ARMATURE:
    {
        exportObject(obj->parent, true);
        if (m_settings.sync_bones) {
            ret = exportArmature(obj, get_path(obj));
        }
        break;
    }
    case OB_MESH:
    {
        exportObject(obj->parent, true);
        if (m_settings.sync_meshes || m_settings.sync_blendshapes) {
            ret = exportMesh(obj, get_path(obj));
        }
        break;
    }
    case OB_CAMERA:
    {
        exportObject(obj->parent, true);
        if (m_settings.sync_cameras) {
            ret = exportCamera(obj, get_path(obj));
        }
        break;
    }
    case OB_LAMP:
    {
        exportObject(obj->parent, true);
        if (m_settings.sync_lights) {
            ret = exportLight(obj, get_path(obj));
        }
        break;
    }
    default:
    {
        if (obj->dup_group || force) {
            exportObject(obj->parent, true);
            ret = exportTransform(obj, get_path(obj));
        }
        break;
    }
    }

    if (ret) {
        exportDupliGroup(obj, ret->path);
        rec.exported = true;
    }

    return ret;
}

ms::TransformPtr msbContext::exportTransform(Object *src, const std::string& path)
{
    if (m_added.find(src) != m_added.end())
        return nullptr;
    m_added.insert(src);

    auto ret = ms::Transform::create();
    auto& dst = *ret;
    dst.path = path;
    ExtractTransformData(src, dst);
    m_entity_manager.add(ret);
    return ret;
}

ms::TransformPtr msbContext::exportPose(bPoseChannel *src, const std::string& path)
{
    auto ret = ms::Transform::create();
    auto& dst = *ret;
    dst.path = path;
    extract_local_TRS(src, dst.position, dst.rotation, dst.scale);
    m_entity_manager.add(ret);
    return ret;
}

ms::TransformPtr msbContext::exportArmature(Object *src, const std::string& path)
{
    if (m_added.find(src) != m_added.end())
        return nullptr;
    m_added.insert(src);

    auto ret = ms::Transform::create();
    auto& dst = *ret;
    dst.path = path;
    ExtractTransformData(src, dst);
    m_entity_manager.add(ret);

    auto poses = bl::list_range((bPoseChannel*)src->pose->chanbase.first);
    for (auto pose : poses) {
        auto bone = pose->bone;
        auto& dst = m_bones[bone];
        dst = exportPose(pose, get_path(src, bone));
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
    auto group = src->dup_group;
    if (!group)
        return nullptr;

    auto local_path = std::string("/") + (group->id.name + 2);
    auto path = base_path + local_path;

    auto dst = ms::Transform::create();
    dst->path = path;
    dst->position = -swap_yz((float3&)group->dupli_ofs);
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

ms::CameraPtr msbContext::exportCamera(Object *src, const std::string& path)
{
    if (m_added.find(src) != m_added.end())
        return nullptr;
    m_added.insert(src);

    auto ret = ms::Camera::create();
    auto& dst = *ret;
    dst.path = path;
    ExtractTransformData(src, dst);
    dst.rotation *= rotateX(90.0f * Deg2Rad);

    ExtractCameraData(src, dst.is_ortho, dst.near_plane, dst.far_plane, dst.fov);
    m_entity_manager.add(ret);
    return ret;
}

ms::LightPtr msbContext::exportLight(Object *src, const std::string& path)
{
    if (m_added.find(src) != m_added.end())
        return nullptr;
    m_added.insert(src);

    auto ret = ms::Light::create();
    auto& dst = *ret;
    dst.path = path;
    ExtractTransformData(src, dst);
    dst.rotation *= rotateX(90.0f * Deg2Rad);

    ExtractLightData(src, dst.light_type, dst.color, dst.intensity, dst.range, dst.spot_angle);
    m_entity_manager.add(ret);
    return ret;
}

ms::MeshPtr msbContext::exportMesh(Object *src, const std::string& path)
{
    // ignore particles
    if (find_modofier(src, eModifierType_ParticleSystem) || find_modofier(src, eModifierType_ParticleInstance))
        return nullptr;

    if (m_added.find(src) != m_added.end())
        return nullptr;
    m_added.insert(src);

    bool is_editing = false;
    bl::BObject bobj(src);
    if (m_settings.sync_meshes) {
        // check if mesh is dirty
        bl::BMesh bmesh(bobj.data());
        if (bmesh.ptr()->edit_btmesh) {
            is_editing = true;
            auto bm = bmesh.ptr()->edit_btmesh->bm;
            if (bm->elem_table_dirty) {
                // mesh is editing and dirty. just add to pending list
                m_pending.insert(src);
                return nullptr;
            }
        }
    }

    auto ret = ms::Mesh::create();
    auto& dst = *ret;
    dst.path = path;

    // transform
    ExtractTransformData(src, dst);

    Mesh *data = (Mesh*)src->data;
    if (m_settings.sync_meshes) {
        // bake modifiers
        if (!is_editing && m_settings.bake_modifiers) {
            // make baked meshes
            data = bobj.to_mesh(bl::BContext::get().scene());
            m_tmp_bmeshes.push_back(data); // need to delete baked meshes later
        }

        // calculate per index normals
        // note: when bake_modifiers is enabled, it is done for baked meshes
        if (m_settings.calc_per_index_normals) {
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

    if(m_settings.dbg_force_single_threaded)
        task();
    else
        m_extract_tasks.push_back(task);
    return ret;
}

void msbContext::doExtractMeshData(ms::Mesh& dst, Object *obj, Mesh *data)
{
    if (m_settings.sync_meshes) {
        bl::BObject bobj(obj);
        bl::BMesh bmesh(data);
        bool is_editing = bmesh.ptr()->edit_btmesh;

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
        mid_table[mi] = getMaterialID(mesh.mat[mi]);
    if (mid_table.empty())
        mid_table.push_back(-1);

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
                        mscTrace("bone not found %s\n", bone->name);
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
    bl::BEditMesh emesh(bmesh.ptr()->edit_btmesh);
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

ms::TransformPtr msbContext::findBone(const Object *armature, const Bone *bone)
{
    auto it = m_bones.find(bone);
    return it != m_bones.end() ? it->second : nullptr;
}

msbContext::ObjectRecord& msbContext::touchRecord(Object * obj)
{
    auto& rec = m_obj_records[obj];
    if (rec.alive)
        return rec; // already touched

    auto path = get_path(obj);
    if (rec.path != path) {
        if (!rec.path.empty()) {
            // obj is renamed
            addDeleted(rec.getIdentifier());
        }
        rec.name = get_name(obj);
        rec.path = path;
    }

    rec.alive = true;
    if (obj->type == OB_ARMATURE) {
        auto poses = bl::list_range((bPoseChannel*)obj->pose->chanbase.first);
        for (auto pose : poses) {
            m_obj_records[pose->bone].alive = true;
        }
    }
    return rec;
}

void msbContext::eraseStaleObjects()
{
    for (auto i = m_obj_records.begin(); i != m_obj_records.end(); /**/) {
        if (!i->second.alive) {
            addDeleted(i->second.getIdentifier());
            m_obj_records.erase(i++);
        }
        else {
            ++i;
        }
    }

    if (!m_deleted.empty()) {
        // blender re-creates all objects when undo / redo.
        // in that case, m_deleted includes all previous objects.
        // to avoid unneeded delete, erase re-created objects from m_deleted.
        m_deleted.erase(std::remove_if(m_deleted.begin(), m_deleted.end(), [this](const ms::Identifier& v) {
            for (auto& kvp : m_obj_records) {
                if (kvp.second.path == v.path)
                    return true;
            }
            return false;
        }), m_deleted.end());
    }
}


void msbContext::sendAnimations(SendScope scope)
{
    // wait previous request to complete
    m_sender.wait();

    m_ignore_update = true;

    auto scene = bl::BScene(bl::BContext::get().scene());
    m_animations.clear();
    m_animations.push_back(ms::AnimationClip::create()); // create default clip

    // list target objects
    if (scope == SendScope::Selected) {
        // todo
    }
    else {
        // all
        for (auto *base : scene.objects()) {
            exportAnimation(base->object, false);
        }
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
        for (int f = frame_start; f <= frame_end; f += interval) {
            scene.frame_set(f);

            m_current_time = frame_to_seconds * f;
            mu::parallel_for_each(m_anim_records.begin(), m_anim_records.end(), [this](AnimationRecords::value_type& kvp) {
                kvp.second(this);
            });
        }
        m_anim_records.clear();
        scene.frame_set(frame_current);
    }

    // keyframe reduction
    for (auto& clip : m_animations) {
        clip->reduction();
    }
    // erase empty clip
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(), [](ms::AnimationClipPtr& p) { return p->empty(); }),
        m_animations.end());

    // send
    if (!m_animations.empty()) {
        kickAsyncSend();
    }

    m_ignore_update = false;
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
    if (force || obj->type == OB_ARMATURE || obj->dup_group) {
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
    if (obj->dup_group) {
        auto group_path = base_path;
        group_path += '/';
        group_path += get_name(obj);
        group_path += '/';
        group_path += (obj->dup_group->id.name + 2);

        auto gobjects = bl::list_range((CollectionObject*)obj->dup_group->gobject.first);
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

    float t = m_current_time * m_settings.animation_timescale;
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

    float t = m_current_time * m_settings.animation_timescale;
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

    float t = m_current_time * m_settings.animation_timescale;
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

    float t = m_current_time * m_settings.animation_timescale;
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
    float t = m_current_time * m_settings.animation_timescale;

    auto& mesh = *(Mesh*)((Object*)obj)->data;
    if (!mesh.edit_btmesh && mesh.key) {
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



bool msbContext::prepare()
{
    if (!bl::ready() || m_sender.isSending())
        return false;

    return true;
}

void msbContext::sendScene(SendScope scope, bool force_all)
{
    if (m_ignore_update || !prepare())
        return;

    if (force_all)
        m_entity_manager.makeDirtyAll();

    if (scope == SendScope::Updated) {
        auto bpy_data = bl::BData(bl::BContext::get().data());
        if (!bpy_data.objects_is_updated())
            return; // nothing to send

        if (m_settings.sync_meshes)
            exportMaterials();

        auto scene = bl::BScene(bl::BContext::get().scene());
        for (auto *base : scene.objects()) {
            auto obj = base->object;
            auto bid = bl::BID(obj);
            if (bid.is_updated() || bid.is_updated_data())
                exportObject(obj, false);
            else
                touchRecord(obj);
        }
    }
    if (scope == SendScope::Selected) {
        // todo
    }
    else {
        // all
        if (m_settings.sync_meshes)
            exportMaterials();

        auto scene = bl::BScene(bl::BContext::get().scene());
        for (auto *base : scene.objects()) {
            exportObject(base->object, false);
        }
    }

    eraseStaleObjects();
    kickAsyncSend();
}

void msbContext::flushPendingList()
{
    if (!m_pending.empty() && !m_sender.isSending()) {
        std::swap(m_pending, m_pending_tmp);
        for (auto p : m_pending_tmp)
            exportObject(p, false);
        m_pending_tmp.clear();
        kickAsyncSend();
    }
}

void msbContext::kickAsyncSend()
{
    // get vertex data in parallel
    parallel_for_each(m_extract_tasks.begin(), m_extract_tasks.end(), [](task_t& task) {
        task();
    });
    m_extract_tasks.clear();

    // clear baked meshes
    if (!m_tmp_bmeshes.empty()) {
        bl::BData bd(bl::BContext::get().data());
        for (auto *v : m_tmp_bmeshes)
            bd.remove(v);
        m_tmp_bmeshes.clear();
    }

    for (auto& kvp : m_obj_records)
        kvp.second.clearState();
    m_bones.clear();
    m_added.clear();

    // kick async send
    if (!m_sender.on_prepare) {
        m_sender.on_prepare = [this]() {
            auto& t = m_sender;
            t.client_settings = m_settings.client_settings;
            t.scene_settings = m_settings.scene_settings;
            float scale_factor = 1.0f / m_settings.scene_settings.scale_factor;
            t.scene_settings.scale_factor = 1.0f;
            t.scene_settings.handedness = ms::Handedness::Left;

            t.textures = m_texture_manager.getDirtyTextures();
            t.materials = m_materials;
            t.transforms = m_entity_manager.getDirtyTransforms();
            t.geometries = m_entity_manager.getDirtyGeometries();
            t.deleted = m_deleted;
            t.animations = m_animations;

            if (scale_factor != 1.0f) {
                for (auto& obj : t.transforms) { obj->applyScaleFactor(scale_factor); }
                for (auto& obj : t.geometries) { obj->applyScaleFactor(scale_factor); }
                for (auto& obj : t.animations) { obj->applyScaleFactor(scale_factor); }
            }
        };
        m_sender.on_succeeded = [this]() {
            m_texture_manager.clearDirtyFlags();
            m_entity_manager.clearDirtyFlags();
            m_deleted.clear();
            m_materials.clear();
            m_animations.clear();
        };
    }
    m_sender.kick();
}
