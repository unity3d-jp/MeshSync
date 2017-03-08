#include "pch.h"
#include "MQSync.h"

#define MaxNameBuffer 128

static inline std::string BuildPath(MQDocument doc, MQObject obj)
{
    std::string ret;
    if (auto parent = doc->GetParentObject(obj)) {
        ret += BuildPath(doc, parent);
    }
    char name[MaxNameBuffer];
    obj->GetName(name, sizeof(name));
    ret += "/";
    ret += name;
    return ret;
}

static inline bool ExtractID(const char *name, int& id)
{
    if (auto p = std::strstr(name, "[id:")) {
        if (sscanf(p, "[id:%08x]", &id) == 1) {
            return true;
        }
    }
    return false;
}


void MQCameraData::get(MQScene scene)
{
    position = (const float3&)scene->GetCameraPosition();
    look_target = (const float3&)scene->GetLookAtPosition();
    angle = (const float3&)scene->GetCameraAngle();
    rotation_center = (const float3&)scene->GetRotationCenter();
    fov = scene->GetFOV();
}

bool MQCameraData::operator==(const MQCameraData& v) const
{
    return position == v.position &&
        look_target == v.look_target &&
        angle == v.angle &&
        rotation_center == v.rotation_center &&
        fov == v.fov;
}

bool MQCameraData::operator!=(const MQCameraData& v) const
{
    return !(*this == v);
}



MQSync::MQSync()
{
}

MQSync::~MQSync()
{
    waitAsyncSend();
}

ms::ClientSettings& MQSync::getClientSettings() { return m_settings; }
float& MQSync::getScaleFactor() { return m_scale_factor; }
bool& MQSync::getAutoSync() { return m_auto_sync; }
bool& MQSync::getBakeSkin() { return m_bake_skin; }
bool& MQSync::getBakeCloth() { return m_bake_cloth; }

void MQSync::clear()
{
    m_client_meshes.clear();
    m_host_meshes.clear();
    m_exist_record.clear();
    m_pending_send_meshes = false;
}

void MQSync::flushPendingRequests(MQDocument doc)
{
    if (m_pending_send_meshes) {
        sendMesh(doc, true);
    }
}

void MQSync::sendMesh(MQDocument doc, bool force)
{
    if (!force)
    {
        if (!m_auto_sync) { return; }
    }

    // just return if previous request is in progress. responsiveness is highest priority.
    if (isAsyncSendInProgress()) {
        m_pending_send_meshes = true;
        return;
    }

    m_pending_send_meshes = false;

    int nobj = doc->GetObjectCount();

    // build relations
    m_obj_for_normals.clear();
    m_relations.clear();
    for (int i = 0; i < nobj; ++i) {
        auto obj = doc->GetObject(i);
        if (!obj) { continue; }

        char name[MaxNameBuffer];
        obj->GetName(name, sizeof(name));

        if (auto pos_normal = std::strstr(name, ":normal")) {
            m_obj_for_normals.push_back(obj);
        }
        else {
            auto rel = Relation();
            rel.obj = obj;
            m_relations.push_back(rel);
        }
    }
    for (auto nobj : m_obj_for_normals) {
        char nname[MaxNameBuffer];
        nobj->GetName(nname, sizeof(nname));
        *std::strstr(nname, ":normal") = '\0';

        for (auto& rel : m_relations) {
            char name[MaxNameBuffer];
            rel.obj->GetName(name, sizeof(name));
            if (strcmp(nname, name) == 0) {
                rel.normal = nobj;
                break;
            }
        }
    }

    while ((int)m_client_meshes.size() < m_relations.size()) {
        m_client_meshes.emplace_back(new ms::Mesh());
    }
    for (size_t i = 0; i < m_relations.size(); ++i) {
        m_relations[i].data = m_client_meshes[i];
        m_relations[i].data->index = (int)i;
    }

    // gather mesh data
    concurrency::parallel_for_each(m_relations.begin(), m_relations.end(), [this, doc](Relation& rel) {
        rel.data->clear();
        rel.data->path = ms::ToUTF8(BuildPath(doc, rel.obj).c_str());
        ExtractID(rel.data->path.c_str(), rel.data->id);

        bool visible = rel.obj->GetVisible() || (rel.normal && rel.normal->GetVisible());
        rel.data->flags.visible = visible;
        if (!visible) {
            // not send actual contents if not visible
            return;
        }

        extractMeshData(doc, rel.obj, *rel.data);
        if (rel.normal) {
            copyPointsForNormalCalculation(doc, rel.normal, *rel.data);
        }
    });


    // gather material data
    int nmat = doc->GetMaterialCount();
    m_materials.clear();
    m_materials.reserve(nmat);
    for (int i = 0; i < nmat; ++i) {
        auto src = doc->GetMaterial(i);
        if (!src) {
            // add dummy material to keep material index
            auto dst = new ms::Material();
            dst->id = -1;
            m_materials.emplace_back(dst);
            continue;
        }

        auto dst = new ms::Material();
        dst->id = src->GetUniqueID();
        {
            char name[128];
            src->GetName(name, sizeof(name));
            dst->name = ms::ToUTF8(name);
        }
        (float3&)dst->color = (const float3&)src->GetColor();
        m_materials.emplace_back(dst);
    }


    // kick async send
    m_future_send = std::async(std::launch::async, [this]() {
        ms::Client client(m_settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::Right;
        scene_settings.scale_factor = m_scale_factor;

        // notify scene begin
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneBegin;
            client.send(fence);
        }

        // send materials
        {
            ms::SetMessage set;
            set.scene.materials = m_materials;
            client.send(set);
        }

        // send meshes one by one to Unity can respond quickly
        concurrency::parallel_for_each(m_relations.begin(), m_relations.end(), [&scene_settings, &client](Relation& rel) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.meshes = { rel.data };
            client.send(set);
        });

        // detect deleted objects and send delete message
        {
            for (auto& e : m_exist_record) {
                e.second = false;
            }
            for (auto& rel : m_relations) {
                if (!rel.data->path.empty()) {
                    m_exist_record[rel.data->path] = true;
                }
            }

            ms::DeleteMessage del;
            for (auto i = m_exist_record.begin(); i != m_exist_record.end(); ) {
                if (!i->second) {
                    int id = 0;
                    ExtractID(i->first.c_str(), id);
                    del.targets.push_back({ i->first , id });
                    m_exist_record.erase(i++);
                }
                else {
                    ++i;
                }
            }
            if (!del.targets.empty()) {
                client.send(del);
            }
        }

        // notify scene end
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneEnd;
            client.send(fence);
        }
    });

}

bool MQSync::importMeshes(MQDocument doc)
{
    waitAsyncSend();

    ms::Client client(m_settings);
    ms::GetMessage gd;
    gd.flags.get_transform = 1;
    gd.flags.get_indices = 1;
    gd.flags.get_points = 1;
    gd.flags.get_uv = 1;
    gd.flags.get_materialIDs = 1;
    gd.scene_settings.handedness = ms::Handedness::Right;
    gd.scene_settings.scale_factor = m_scale_factor;
    gd.refine_settings.flags.apply_local2world = 1;
    gd.refine_settings.flags.invert_v = 1;
    gd.refine_settings.flags.bake_skin = m_bake_skin;
    gd.refine_settings.flags.bake_cloth = m_bake_cloth;

    auto ret = client.send(gd);
    if (!ret) {
        return false;
    }

    // import materials
    {
        // create unique name list as Metasequoia doesn't accept multiple same names
        std::vector<std::string> names;
        names.reserve(ret->materials.size());
        for (int i = 0; i < (int)ret->materials.size(); ++i) {
            auto name = ret->materials[i]->name;
            while (std::find(names.begin(), names.end(), name) != names.end()) {
                name += '_';
            }
            names.push_back(name);
        }

        while (doc->GetMaterialCount() < (int)ret->materials.size()) {
            doc->AddMaterial(MQ_CreateMaterial());
        }
        for (int i = 0; i < (int)ret->materials.size(); ++i) {
            auto dst = doc->GetMaterial(i);
            dst->SetName(ms::ToANSI(names[i]).c_str());
            dst->SetColor((const MQColor&)ret->materials[i]->color);
        }
    }
    
    // import meshes
    for (auto& data : ret->meshes) {
        auto& mdata = *data;

        // create name that includes ID
        char name[MaxNameBuffer];
        sprintf(name, "%s [id:%08x]", ms::ToANSI(mdata.getName()).c_str(), mdata.id);

        if (auto obj = findMesh(doc, name)) {
            doc->DeleteObject(doc->GetObjectIndex(obj));
        }
        auto obj = createMesh(mdata, name);
        doc->AddObject(obj);

        m_host_meshes[mdata.id] = data;
    }
    return true;
}

bool MQSync::isAsyncSendInProgress()
{
    if (m_future_send.valid()) {
        return m_future_send.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
    }
    return false;
}

void MQSync::waitAsyncSend()
{
    if (m_future_send.valid()) {
        m_future_send.get();
    }
}

MQObject MQSync::findMesh(MQDocument doc, const char *name)
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

MQObject MQSync::createMesh(const ms::Mesh& data, const char *name)
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
    if(!data.uv.empty()) {
        float2 uv[3];
        size_t nfaces = data.indices.size() / 3;
        for (size_t i = 0; i < nfaces; ++i) {
            uv[0] = data.uv[data.indices[i * 3 + 0]];
            uv[1] = data.uv[data.indices[i * 3 + 1]];
            uv[2] = data.uv[data.indices[i * 3 + 2]];
            ret->SetFaceCoordinateArray((int)i, (MQCoordinate*)uv);
        }
    }
    if (!data.materialIDs.empty()) {
        size_t nfaces = data.indices.size() / 3;
        for (size_t i = 0; i < nfaces; ++i) {
            ret->SetFaceMaterial((int)i, data.materialIDs[i]);
        }
    }
    return ret;
}

void MQSync::extractMeshData(MQDocument doc, MQObject obj, ms::Mesh& dst)
{
    dst.flags.has_points = 1;
    dst.flags.has_uv = 1;
    dst.flags.has_counts = 1;
    dst.flags.has_indices = 1;
    dst.flags.has_materialIDs = 1;
    dst.flags.has_refine_settings = 1;

    dst.refine_settings.flags.gen_normals_with_smooth_angle = 1;
    dst.refine_settings.smooth_angle = obj->GetSmoothAngle();
    dst.refine_settings.flags.gen_tangents = 1;
    dst.refine_settings.flags.invert_v = 1;
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
        auto ang = obj->GetRotation();
        auto eular = float3{ ang.pitch, ang.head, ang.bank } * mu::Deg2Rad;
        quatf rot = rotateZXY(eular);

        dst.flags.apply_trs = 1;
        dst.transform.position = (const float3&)obj->GetTranslation();
        dst.transform.rotation = rot;
        dst.transform.scale = (const float3&)obj->GetScaling();

        dst.refine_settings.flags.apply_world2local = 1;
        dst.refine_settings.world2local = (float4x4&)obj->GetLocalInverseMatrix();

        auto ite = m_host_meshes.find(dst.id);
        if (ite != m_host_meshes.end()) {
            dst.refine_settings.world2local = ite->second->refine_settings.world2local;
            dst.flags.apply_trs = 0;
        }
    }

    // copy vertices
    int npoints = obj->GetVertexCount();
    dst.points.resize(npoints);
    obj->GetVertexArray((MQPoint*)dst.points.data());

    // copy faces
    int nfaces = obj->GetFaceCount();
    int nindices = 0;
    dst.counts.resize(nfaces);
    for (int fi = 0; fi < nfaces; ++fi) {
        int c = obj->GetFacePointCount(fi);
        dst.counts[fi] = c;
        if (c >= 3) { // ignore lines and points
            nindices += c;
        }
    }

    // copy indices, uv, material ID
    dst.indices.resize(nindices);
    dst.uv.resize(nindices);
    dst.materialIDs.resize(nfaces);
    auto *indices = dst.indices.data();
    auto *uv = dst.uv.data();
    for (int fi = 0; fi < nfaces; ++fi) {
        int c = dst.counts[fi];
        dst.materialIDs[fi] = c < 3 ? -2 : obj->GetFaceMaterial(fi); // assign -2 for lines and points and erase later
        if (c >= 3 /*&& obj->GetFaceVisible(fi)*/) {
            obj->GetFacePointArray(fi, indices);
            obj->GetFaceCoordinateArray(fi, (MQCoordinate*)uv);
            indices += c;
            uv += c;
        }
    }

    // copy vertex colors if needed
    {
        bool copy_vertex_color = false;

        auto mids = dst.materialIDs;
        mids.erase(std::unique(mids.begin(), mids.end()), mids.end());
        for (int mid : mids) {
            if (mid >= 0) {
                if (doc->GetMaterial(mid)->GetVertexColor() != MQMATERIAL_VERTEXCOLOR_DISABLE) {
                    copy_vertex_color = true;
                    break;
                }
            }
        }

        if (copy_vertex_color) {
            dst.colors.resize(nindices);
            dst.flags.has_colors = 1;

            auto *colors = dst.colors.data();
            for (int fi = 0; fi < nfaces; ++fi) {
                int count = dst.counts[fi];
                if (count >= 3 /*&& obj->GetFaceVisible(fi)*/) {
                    for (int ci = 0; ci < count; ++ci) {
                        auto color = obj->GetFaceVertexColor(fi, ci);
                        *(colors++) = {
                            (float)(color >> 24) / 255.0f,
                            (float)((color & 0xff0000) >> 16) / 255.0f,
                            (float)((color & 0xff00) >> 8) / 255.0f,
                            (float)(color & 0xff) / 255.0f,
                        };
                    }
                }
            }
        }
    }

    // remove line and points
    dst.materialIDs.erase(
        std::remove(dst.materialIDs.begin(), dst.materialIDs.end(), -2),
        dst.materialIDs.end());
    dst.counts.erase(
        std::remove_if(dst.counts.begin(), dst.counts.end(), [](int c) { return c < 3; }),
        dst.counts.end());
}

void MQSync::copyPointsForNormalCalculation(MQDocument doc, MQObject obj, ms::Mesh& dst)
{
    int npoints = obj->GetVertexCount();
    dst.npoints.resize(npoints);
    obj->GetVertexArray((MQPoint*)dst.npoints.data());
    dst.flags.has_npoints = 1;
}

bool MQSync::syncCameras(MQDocument doc)
{
    bool ret = false;
    for (int i = 0; i < 4; ++i) {
        auto scene = doc->GetScene(i);
        if (!scene) { break; }

        MQCameraData cd;
        cd.get(scene);
        if (m_cameras[i] != cd) {
            ret = true;
            m_cameras[i] = cd;
        }
    }
    return ret;
}
