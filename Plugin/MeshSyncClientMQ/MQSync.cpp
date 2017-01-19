#include "pch.h"
#include "MQSync.h"

#define MaxNameBuffer 1024

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

void MQSync::clear()
{
    m_export_objects.clear();
    m_import_objects.clear();
    m_pending_send_mesh = false;
}

void MQSync::flushPendingRequests(MQDocument doc)
{
    if (m_pending_send_mesh) {
        sendMesh(doc, true);
    }
}

void MQSync::sendMesh(MQDocument doc, bool force)
{
    if (!force && !m_auto_sync) { return; }

    // just return if previous request is in progress. responsiveness is highest priority.
    if (isAsyncSendInProgress()) {
        m_pending_send_mesh = true;
        return;
    }

    m_pending_send_mesh = false;

    int nobj = doc->GetObjectCount();
    while ((int)m_export_objects.size() < nobj) {
        m_export_objects.emplace_back(new ms::MeshData());
    }

    // gather mesh data
    concurrency::parallel_for(0, nobj, [this, doc](int i) {
        extractMeshData(doc, doc->GetObject(i), *m_export_objects[i]);
    });

    // kick async send
    m_future_send = std::async(std::launch::async, [this, nobj]() {
        ms::Client client(m_settings);

        // send mesh
        concurrency::parallel_for(0, nobj, [this, &client](int i) {
            client.send(*m_export_objects[i]);
        });

        // detect deleted objects and send delete message
        for (auto& e : m_exist_record) {
            e.second = false;
        }
        for (int i = 0; i < nobj; ++i) {
            m_exist_record[m_export_objects[i]->path] = true;
        }
        for (auto i = m_exist_record.begin(); i != m_exist_record.end(); ) {
            if (!i->second) {
                ms::DeleteData del;
                del.path = i->first;
                client.send(del);
                m_exist_record.erase(i++);
            }
            else {
                ++i;
            }
        }
    });

}

void MQSync::importMeshes(MQDocument doc)
{
    waitAsyncSend();

    ms::Client client(m_settings);
    ms::GetData gd;
    gd.flags.get_transform = 1;
    gd.flags.get_indices = 1;
    gd.flags.get_points = 1;
    gd.flags.get_uv = 1;
    gd.flags.swap_handedness = 1;
    gd.flags.apply_local2world = 1;
    gd.flags.bake_skin = 1;
    gd.scale = 1.0f / m_scale_factor;

    auto ret = client.send(gd);
    for (auto& data : ret) {
        auto& mdata = *data;

        char name[MaxNameBuffer];
        sprintf(name, "%s [id:%08x]", mdata.getName(), mdata.id_unity);

        if (auto obj = findMQObject(doc, name)) {
            doc->DeleteObject(doc->GetObjectIndex(obj));
        }
        auto obj = createObject(mdata, name);
        doc->AddObject(obj);

        m_import_objects[mdata.id_unity] = data;
    }
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

MQObject MQSync::findMQObject(MQDocument doc, const char *name)
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

MQObject MQSync::createObject(const ms::MeshData& data, const char *name)
{
    auto ret = MQ_CreateObject();

    ret->SetName(name);
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
    return ret;
}

void MQSync::extractMeshData(MQDocument doc, MQObject obj, ms::MeshData& dst)
{
    if (!obj) { return; }

    dst.sender = ms::SenderType::Metasequoia;
    dst.path = BuildPath(doc, obj);
    dst.id_dcc = doc->GetObjectIndex(obj);

    dst.flags.visible = obj->GetVisible();
    if (!dst.flags.visible) {
        // not send actual contents if not visible
        return;
    }

    dst.flags.has_points = 1;
    dst.flags.has_uv = 1;
    dst.flags.has_counts = 1;
    dst.flags.has_indices = 1;
    dst.flags.has_materialIDs = 1;
    dst.flags.has_transform = 1;
    dst.flags.has_refine_settings = 1;

    dst.refine_settings.scale_factor = m_scale_factor;
    dst.refine_settings.flags.swap_handedness = 1;
    dst.refine_settings.flags.gen_normals_with_smooth_angle = 1;
    dst.refine_settings.smooth_angle = obj->GetSmoothAngle();
    dst.refine_settings.flags.gen_tangents = 1;
    dst.refine_settings.flags.invert_v = 1;
    if (obj->GetMirrorType() != MQOBJECT_MIRROR_NONE) {
        int axis = obj->GetMirrorAxis();
        dst.refine_settings.flags.mirror_x = (axis & MQOBJECT_MIRROR_AXIS_X) ? 1 : 0;
        dst.refine_settings.flags.mirror_y = (axis & MQOBJECT_MIRROR_AXIS_Y) ? 1 : 0;
        dst.refine_settings.flags.mirror_z = (axis & MQOBJECT_MIRROR_AXIS_Z) ? 1 : 0;
    }

    // transform
    {
        auto ang = obj->GetRotation();
        auto eular = float3{ ang.pitch, ang.head, ang.bank } *mu::Deg2Rad;
        quatf rot = rotateZXY(eular);

        dst.transform.position = (const float3&)obj->GetTranslation();
        dst.transform.rotation = rot;
        dst.transform.scale = (const float3&)obj->GetScaling();

        auto pos_id = dst.path.find_first_of("[id:");
        if (pos_id != std::string::npos) {
            int id = 0;
            if (sscanf(&dst.path[pos_id], "[id:%x]", &id) == 1) {
                auto ite = m_import_objects.find(id);
                if (ite != m_import_objects.end()) {
                    dst.refine_settings.flags.apply_world2local = 1;
                    dst.transform.local2world = ite->second->transform.local2world;
                    dst.transform.world2local = ite->second->transform.world2local;
                }
            }
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

    // remove line and points
    dst.materialIDs.erase(
        std::remove(dst.materialIDs.begin(), dst.materialIDs.end(), -2),
        dst.materialIDs.end());
    dst.counts.erase(
        std::remove_if(dst.counts.begin(), dst.counts.end(), [](int c) { return c < 3; }),
        dst.counts.end());

    // Metasequoia uses -1 as invalid material. +1 to make it zero-based
    for (int& mid : dst.materialIDs) {
        mid += 1;
    }
}
