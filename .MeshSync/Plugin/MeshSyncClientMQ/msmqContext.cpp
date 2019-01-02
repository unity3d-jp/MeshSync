#include "pch.h"
#include "msmqContext.h"
#include "msmqUtils.h"


msmqContext::msmqContext(MQBasePlugin *plugin)
{
    m_plugin = plugin;
}

msmqContext::~msmqContext()
{
    m_send_meshes.wait();
    m_send_camera.wait();
}

msmqSettings& msmqContext::getSettings()
{
    return m_settings;
}

void msmqContext::clear()
{
    m_obj_records.clear();
    m_bone_records.clear();
    m_host_meshes.clear();

    m_texture_manager.clear();
    m_material_manager.clear();
    m_entity_manager.clear();
    m_camera.reset();
    m_pending_send_meshes = false;
}

void msmqContext::flushPendingRequests(MQDocument doc)
{
    if (m_pending_send_meshes) {
        sendMeshes(doc, true);
    }
}

void msmqContext::sendMeshes(MQDocument doc, bool force)
{
    if (!force) {
        if (!m_settings.auto_sync)
            return;
    }

    if (m_send_meshes.isSending()) {
        if (force) {
            m_send_meshes.wait();
        }
        else {
            m_pending_send_meshes = true;
            return;
        }
    }
    m_pending_send_meshes = false;

    if (force) {
        m_material_manager.makeDirtyAll();
        m_entity_manager.makeDirtyAll();
    }

    {
        // gather material data
        char buf[1024];
        int nmat = doc->GetMaterialCount();

        m_material_index_to_id.clear();
        m_material_index_to_id.resize(nmat, ms::InvalidID);

        for (int mi = 0; mi < nmat; ++mi) {
            auto src = doc->GetMaterial(mi);
            if (!src)
                continue;

            auto dst = ms::Material::create();
            dst->id = m_material_index_to_id[mi] = m_material_ids.getID(src);
            dst->index = mi;
            src->GetName(buf, sizeof(buf));
            dst->name = ms::ToUTF8(buf);

            auto& stdmat = ms::AsStandardMaterial(*dst);
            stdmat.setColor(to_float4(src->GetColor()));
            if (m_settings.sync_textures) {
                src->GetTextureName(buf, sizeof(buf));
                stdmat.setColorMap(exportTexture(buf, ms::TextureType::Default));
            }
            m_material_manager.add(dst);
        }
    }

    {
        // gather meshes
        int nobj = doc->GetObjectCount();
        for (int i = 0; i < nobj; ++i) {
            auto obj = doc->GetObject(i);
            if (!obj)
                continue;

            ObjectRecord rel;
            rel.obj = obj;
            rel.dst = ms::Mesh::create();
            rel.dst->index = (int)i;
            m_obj_records.push_back(std::move(rel));
        }

        // extract mesh data
        parallel_for_each(m_obj_records.begin(), m_obj_records.end(), [this, doc](ObjectRecord& rec) {
            rec.dst->path = ms::ToUTF8(BuildPath(doc, rec.obj).c_str());
            ExtractID(rec.dst->path.c_str(), rec.dst->id);

            bool visible = rec.obj->GetVisible() != 0;
            rec.dst->visible = visible;
            if (visible)
                extractMeshData(doc, rec.obj, *rec.dst);
            // add to m_entity_manager will be done later because bone weights affect checksum
        });
    }

#if MQPLUGIN_VERSION >= 0x0464
    if (m_settings.sync_bones) {
        // gather bone data
        MQBoneManager bone_manager(m_plugin, doc);

        int nbones = bone_manager.GetBoneNum();
        if (nbones == 0) {
            goto bone_end;
        }

        {
            std::wstring name;
            std::vector<UINT> bone_ids;

            bone_manager.EnumBoneID(bone_ids);
            for (auto bid : bone_ids) {
                auto& brec = m_bone_records[bid];
                brec.bone_id = bid;

                bone_manager.GetName(bid, name);
                brec.name = ToMBS(name);

                UINT parent;
                bone_manager.GetParent(bid, parent);
                brec.parent_id = parent;

                MQPoint base_pos;
                bone_manager.GetBaseRootPos(bid, base_pos);
                brec.world_pos = (const float3&)base_pos;
                brec.bindpose = mu::invert(mu::transform(brec.world_pos, quatf::identity(), float3::one()));

                if (m_settings.sync_poses) {
                    MQMatrix rot;
                    bone_manager.GetRotationMatrix(bid, rot);
                    brec.world_rot = mu::invert(mu::to_quat((float4x4&)rot));
                }
                else {
                    brec.world_rot = quatf::identity();
                }
            }

            for (auto& kvp : m_bone_records) {
                auto& rec = kvp.second;

                // build path
                auto& path = rec.dst->path;
                path = "/bones";
                buildBonePath(path, rec);

                // setup transform
                auto& dst = *rec.dst;
                dst.rotation = rec.world_rot;
                dst.position = rec.world_pos;
                auto it = m_bone_records.find(rec.parent_id);
                if (it != m_bone_records.end())
                    dst.position -= it->second.world_pos;
            }

            // get weights
            parallel_for_each(m_obj_records.begin(), m_obj_records.end(), [this, &bone_manager, &bone_ids](ObjectRecord& rec) {
                auto obj = rec.obj;
                if (!rec.dst->visible)
                    return;

                std::vector<UINT> vertex_ids;
                std::vector<float> weights;
                for (auto bid : bone_ids) {
                    int nweights = bone_manager.GetWeightedVertexArray(bid, obj, vertex_ids, weights);
                    if (nweights == 0)
                        continue;

                    // find root bone
                    if (rec.dst->root_bone.empty()) {
                        auto it = m_bone_records.find(bid);
                        for (;;) {
                            auto next = m_bone_records.find(it->second.parent_id);
                            if (next == m_bone_records.end())
                                break;
                            it = next;
                        }
                        rec.dst->root_bone = it->second.dst->path;
                    }

                    auto& bone = m_bone_records[bid];
                    auto bd = ms::BoneData::create();
                    rec.dst->bones.push_back(bd);
                    rec.dst->flags.has_bones = 1;
                    bd->path = bone.dst->path;
                    bd->bindpose = bone.bindpose;

                    auto size = rec.dst->points.size();
                    bd->weights.resize_zeroclear(size);
                    for (int iw = 0; iw < nweights; ++iw) {
                        int vi = obj->GetVertexIndexFromUniqueID(vertex_ids[iw]);
                        if (vi >= 0)
                            bd->weights[vi] = weights[iw];
                    }
                }
            });
        }
    bone_end:;
    }
#endif

    for (auto& rec : m_bone_records)
        m_entity_manager.add(rec.second.dst);
    m_bone_records.clear();

    for (auto& rec : m_obj_records)
        m_entity_manager.add(rec.dst);
    m_obj_records.clear();

    m_material_ids.eraseStaleRecords();
    m_material_manager.eraseStaleMaterials();
    m_entity_manager.eraseStaleEntities();

    m_send_meshes.on_prepare = [this]() {
        auto& t = m_send_meshes;
        t.client_settings = m_settings.client_settings;
        t.scene_settings.handedness = ms::Handedness::Right;
        t.scene_settings.scale_factor = m_settings.scale_factor;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();

        t.deleted_materials = m_material_manager.getDeleted();
        t.deleted_entities = m_entity_manager.getDeleted();
    };
    m_send_meshes.on_success = [this]() {
        m_material_ids.clearDirtyFlags();
        m_texture_manager.clearDirtyFlags();
        m_material_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
    };
    m_send_meshes.kick();
}

void msmqContext::buildBonePath(std::string& dst, BoneRecord& bd)
{
    auto it = m_bone_records.find(bd.parent_id);
    if (it != m_bone_records.end())
        buildBonePath(dst, it->second);

    dst += "/";
    dst += bd.name;
}


void msmqContext::sendCamera(MQDocument doc, bool force)
{
    if (!force) {
        if (!m_settings.auto_sync)
            return;
    }
    if (!m_settings.sync_camera)
        return;

    // just return if previous request is in progress
    if (m_send_camera.isSending())
        return;

    // gather camera data
    if (auto scene = doc->GetScene(0)) { // GetScene(0): perspective view
        if (!m_camera) {
            m_camera = ms::Camera::create();
            m_camera->near_plane *= m_settings.scale_factor;
            m_camera->far_plane *= m_settings.scale_factor;
        }
        auto prev_pos = m_camera->position;
        auto prev_rot = m_camera->rotation;
        auto prev_fov = m_camera->fov;

        m_camera->path = m_settings.host_camera_path;
        extractCameraData(doc, scene, *m_camera);

        if (!force &&
            m_camera->position == prev_pos &&
            m_camera->rotation == prev_rot &&
            m_camera->fov == prev_fov)
        {
            // no need to send
            return;
        }
    }

    if (!m_send_camera.on_prepare) {
        m_send_camera.on_prepare = [this]() {
            auto& t = m_send_camera;
            t.client_settings = m_settings.client_settings;
            t.scene_settings.handedness = ms::Handedness::Right;
            t.scene_settings.scale_factor = m_settings.scale_factor;
            t.transforms = { m_camera };
        };
    }
    m_send_camera.kick();
}


bool msmqContext::importMeshes(MQDocument doc)
{
    ms::Client client(m_settings.client_settings);
    ms::GetMessage gd;
    gd.flags.get_transform = 1;
    gd.flags.get_indices = 1;
    gd.flags.get_points = 1;
    gd.flags.get_uv0 = 1;
    gd.flags.get_colors = 1;
    gd.flags.get_material_ids = 1;
    gd.scene_settings.handedness = ms::Handedness::Right;
    gd.scene_settings.scale_factor = m_settings.scale_factor;
    gd.refine_settings.flags.apply_local2world = 1;
    gd.refine_settings.flags.flip_v = 1;
    gd.refine_settings.flags.bake_skin = m_settings.bake_skin;
    gd.refine_settings.flags.bake_cloth = m_settings.bake_cloth;

    auto ret = client.send(gd);
    if (!ret) {
        return false;
    }

    // import materials
    {
        auto material = ret->getAssets<ms::Material>();

        // create unique name list as Metasequoia doesn't accept multiple same names
        std::vector<std::string> names;
        names.reserve(material.size());
        for (int i = 0; i < (int)material.size(); ++i) {
            auto name = material[i]->name;
            while (std::find(names.begin(), names.end(), name) != names.end()) {
                name += '_';
            }
            names.push_back(name);
        }

        while (doc->GetMaterialCount() < (int)material.size()) {
            doc->AddMaterial(MQ_CreateMaterial());
        }
        for (int i = 0; i < (int)material.size(); ++i) {
            auto dst = doc->GetMaterial(i);
            dst->SetName(ms::ToANSI(names[i]).c_str());

            auto& stdmat = ms::AsStandardMaterial(*material[i]);
            dst->SetColor(to_MQColor(stdmat.getColor()));
        }
    }
    
    // import meshes
    for (auto& data : ret->entities) {
        if (data->getType() == ms::Entity::Type::Mesh) {
            auto& mdata = (ms::Mesh&)*data;

            // create name that includes ID
            char name[MaxNameBuffer];
            sprintf(name, "%s [id:%08x]", ms::ToANSI(mdata.getName()).c_str(), mdata.id);

            if (auto obj = findMesh(doc, name)) {
                doc->DeleteObject(doc->GetObjectIndex(obj));
            }
            auto obj = createMesh(doc, mdata, name);
            doc->AddObject(obj);

            m_host_meshes[mdata.id] = std::static_pointer_cast<ms::Mesh>(data);
        }
    }
    return true;
}

int msmqContext::exportTexture(const std::string& path, ms::TextureType type)
{
    return m_texture_manager.addFile(path, type);
}

MQObject msmqContext::findMesh(MQDocument doc, const char *name)
{
    int nobj = doc->GetObjectCount();
    for (int i = 0; i < nobj; ++i) {
        auto obj = doc->GetObject(i);
        if (!obj) { continue; }

        char tmp[MaxNameBuffer];
        obj->GetName(tmp, sizeof(tmp));
        if (strcmp(tmp, name) == 0) {
            return obj;
        }
    }
    return nullptr;
}

MQObject msmqContext::createMesh(MQDocument doc, const ms::Mesh& data, const char *name)
{
    auto ret = MQ_CreateObject();

    ret->SetName(name);

    // gave up importing transform

    ret->SetSmoothAngle(data.refine_settings.smooth_angle);

    for (auto& p : data.points) {
        ret->AddVertex((MQPoint&)p);
    }
    {
        size_t nindices = data.indices.size();
        for (size_t i = 0; i < nindices; i += 3) {
            ret->AddFace(3, const_cast<int*>(&data.indices[i]));
        }
    }
    if(!data.uv0.empty()) {
        float2 uv[3];
        size_t nfaces = data.indices.size() / 3;
        for (size_t i = 0; i < nfaces; ++i) {
            uv[0] = data.uv0[data.indices[i * 3 + 0]];
            uv[1] = data.uv0[data.indices[i * 3 + 1]];
            uv[2] = data.uv0[data.indices[i * 3 + 2]];
            ret->SetFaceCoordinateArray((int)i, (MQCoordinate*)uv);
        }
    }
    if (!data.colors.empty()) {
        size_t nfaces = data.indices.size() / 3;
        for (size_t i = 0; i < nfaces; ++i) {
            ret->SetFaceVertexColor((int)i, 0, mu::Float4ToColor32(data.colors[data.indices[i * 3 + 0]]));
            ret->SetFaceVertexColor((int)i, 1, mu::Float4ToColor32(data.colors[data.indices[i * 3 + 1]]));
            ret->SetFaceVertexColor((int)i, 2, mu::Float4ToColor32(data.colors[data.indices[i * 3 + 2]]));
        }
        // enable vertex color flag on assigned materials
        auto mids = data.material_ids;
        mids.erase(std::unique(mids.begin(), mids.end()), mids.end());
        for (auto mid : mids) {
            if (mid >= 0) {
                doc->GetMaterial(mid)->SetVertexColor(MQMATERIAL_VERTEXCOLOR_DIFFUSE);
            }
        }
    }
    if (!data.material_ids.empty()) {
        size_t nfaces = data.indices.size() / 3;
        for (size_t i = 0; i < nfaces; ++i) {
            ret->SetFaceMaterial((int)i, data.material_ids[i]);
        }
    }
    return ret;
}

void msmqContext::extractMeshData(MQDocument doc, MQObject obj, ms::Mesh& dst)
{
    dst.flags.has_refine_settings = 1;
    dst.refine_settings.flags.make_double_sided = m_settings.make_double_sided;
    dst.refine_settings.flags.gen_tangents = 1;
    dst.refine_settings.flags.flip_v = 1;
    if (obj->GetMirrorType() != MQOBJECT_MIRROR_NONE) {
        int axis = obj->GetMirrorAxis();
        dst.refine_settings.flags.mirror_x = (axis & MQOBJECT_MIRROR_AXIS_X) ? 1 : 0;
        dst.refine_settings.flags.mirror_y = (axis & MQOBJECT_MIRROR_AXIS_Y) ? 1 : 0;
        dst.refine_settings.flags.mirror_z = (axis & MQOBJECT_MIRROR_AXIS_Z) ? 1 : 0;
        if (obj->GetMirrorType() == MQOBJECT_MIRROR_JOIN) {
            dst.refine_settings.flags.mirror_x_weld = dst.refine_settings.flags.mirror_x;
            dst.refine_settings.flags.mirror_y_weld = dst.refine_settings.flags.mirror_y;
            dst.refine_settings.flags.mirror_z_weld = dst.refine_settings.flags.mirror_z;
        }
    }

    // transform
    {
        dst.refine_settings.flags.apply_world2local = 1;
        auto ite = m_host_meshes.find(dst.id);
        if (ite != m_host_meshes.end()) {
            dst.refine_settings.world2local = ite->second->refine_settings.world2local;
            dst.flags.apply_trs = 0;
        }
        else {
            dst.flags.apply_trs = 1;
            ExtractLocalTransform(obj, dst.position, dst.rotation, dst.scale);
            dst.refine_settings.world2local = invert(ExtractGlobalMatrix(doc, obj));
        }
    }

    // vertices
    int npoints = obj->GetVertexCount();
    dst.points.resize(npoints);
    obj->GetVertexArray((MQPoint*)dst.points.data());

    // faces
    int nfaces = obj->GetFaceCount();
    int nindices = 0;
    dst.counts.resize(nfaces);
    for (int fi = 0; fi < nfaces; ++fi) {
        int c = obj->GetFacePointCount(fi);
        dst.counts[fi] = c;
        nindices += c;
    }

    // indices, uv, material ID
    dst.indices.resize_discard(nindices);
    dst.uv0.resize_discard(nindices);
    dst.material_ids.resize_discard(nfaces);
    auto *indices = dst.indices.data();
    auto *uv = dst.uv0.data();
    for (int fi = 0; fi < nfaces; ++fi) {
        int material_index = obj->GetFaceMaterial(fi);
        dst.material_ids[fi] = material_index != -1 ? m_material_index_to_id[material_index] : ms::InvalidID;

        int c = dst.counts[fi];
        //if (obj->GetFaceVisible(fi))
        {
            obj->GetFacePointArray(fi, indices);
            obj->GetFaceCoordinateArray(fi, (MQCoordinate*)uv);
            indices += c;
            uv += c;
        }
    }

    // vertex colors
    if (m_settings.sync_vertex_color) {
        dst.colors.resize_discard(nindices);
        dst.flags.has_colors = 1;
        auto *colors = dst.colors.data();
        for (int fi = 0; fi < nfaces; ++fi) {
            int count = dst.counts[fi];
            //if (obj->GetFaceVisible(fi))
            {
                for (int ci = 0; ci < count; ++ci) {
                    *(colors++) = Color32ToFloat4(obj->GetFaceVertexColor(fi, ci));
                }
            }
        }
    }

    // normals
#if MQPLUGIN_VERSION >= 0x0460
    if (m_settings.sync_normals) {
        dst.normals.resize_discard(nindices);
        dst.flags.has_normals = 1;
        auto *normals = dst.normals.data();
        for (int fi = 0; fi < nfaces; ++fi) {
            int count = dst.counts[fi];
            BYTE flags;
            //if (obj->GetFaceVisible(fi))
            {
                for (int ci = 0; ci < count; ++ci) {
                    obj->GetFaceVertexNormal(fi, ci, flags, (MQPoint&)*(normals++));
                }
            }
        }
    }
    else
#endif
    {
        dst.refine_settings.flags.gen_normals_with_smooth_angle = 1;
        dst.refine_settings.smooth_angle = obj->GetSmoothAngle();
    }
    dst.setupFlags();
}

void msmqContext::extractCameraData(MQDocument doc, MQScene scene, ms::Camera& dst)
{
    dst.position = to_float3(scene->GetCameraPosition());
    auto eular = ToEular(scene->GetCameraAngle(), true);
    dst.rotation = rotateZXY(eular);

    dst.fov = scene->GetFOV() * mu::Rad2Deg;
#if MQPLUGIN_VERSION >= 0x450
    dst.near_plane = scene->GetFrontClip();
    dst.far_plane = scene->GetBackClip();
#endif
}
