#include "pch.h"
#include "Sync.h"

#define MaxNameBuffer 1024

Sync::Sync()
{
}

Sync::~Sync()
{
    waitAsyncSend();
}

ms::ClientSettings& Sync::getClientSettings() { return m_settings; }
float& Sync::getScaleFactor() { return m_scale_factor; }
bool& Sync::getAutoSync() { return m_auto_sync; }

void Sync::send(MQDocument doc, bool force)
{
    if (!force && !m_auto_sync) { return; }

    waitAsyncSend();

    int nobj = doc->GetObjectCount();
    while ((int)m_data.size() < nobj) {
        m_data.emplace_back(new ms::MeshData());
    }

    // gather mesh data
    concurrency::parallel_for(0, nobj, [this, doc](int i) {
        gather(doc, doc->GetObject(i), *m_data[i]);
    });


    // kick async send
    m_future_send = std::async(std::launch::async, [this, nobj]() {
        ms::Client client(m_settings);

        // send mesh
        concurrency::parallel_for(0, nobj, [this, &client](int i) {
            client.send(*m_data[i]);
        });

        // detect deleted objects
        m_prev_objects.swap(m_current_objects);
        m_current_objects.resize(nobj);
        for (int i = 0; i < nobj; ++i) {
            m_current_objects[i] = m_data[i]->path;
        }
        std::sort(m_current_objects.begin(), m_current_objects.end());

        std::vector<ms::DeleteData> del_data;
        for (auto& n : m_prev_objects) {
            if (std::lower_bound(m_current_objects.begin(), m_current_objects.end(), n) == m_current_objects.end()) {
                ms::DeleteData data;
                data.path = n;
                del_data.push_back(data);
            }
        }

        // send delete event
        if (!del_data.empty()) {
            std::vector<ms::DeleteData*> send_data(del_data.size());
            for (size_t i = 0; i < del_data.size(); ++i) { send_data[i] = &del_data[i]; }

            client.send(send_data.data(), (int)send_data.size());
        }
    });

}

void Sync::import(MQDocument doc)
{
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
        auto& mdata = static_cast<ms::MeshData&>(*data);

        char name[MaxNameBuffer];
        sprintf(name, "%s [id:%08x]", mdata.getName(), mdata.id);

        if (auto obj = findObject(doc, name)) {
            doc->DeleteObject(doc->GetObjectIndex(obj));
        }
        auto obj = createObject(mdata, name);
        doc->AddObject(obj);
    }
}

void Sync::waitAsyncSend()
{
    if (m_future_send.valid()) {
        m_future_send.get();
    }
}

MQObject Sync::findObject(MQDocument doc, const char *name)
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

MQObject Sync::createObject(const ms::MeshData& data, const char *name)
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

static inline void BuildPath(MQDocument doc, MQObject obj, std::string& path)
{
    if (auto parent = doc->GetParentObject(obj)) {
        BuildPath(doc, parent, path);
    }
    char name[MaxNameBuffer];
    obj->GetName(name, sizeof(name));
    path += "/";
    path += name;
}

void Sync::gather(MQDocument doc, MQObject obj, ms::MeshData& dst)
{
    if (!obj) { return; }

    dst.path.clear();
    BuildPath(doc, obj, dst.path);

    // metasequoia specific attributes
    {
        dst.csd.type = ms::ClientSpecificData::Type::Metasequoia;
        dst.csd.mq.smooth_angle = obj->GetSmoothAngle();
        dst.csd.mq.flags.invert_v = 1;

        if (obj->GetMirrorType() != MQOBJECT_MIRROR_NONE) {
            int axis = obj->GetMirrorAxis();
            dst.csd.mq.flags.mirror_x = (axis & MQOBJECT_MIRROR_AXIS_X) ? 1 : 0;
            dst.csd.mq.flags.mirror_y = (axis & MQOBJECT_MIRROR_AXIS_Y) ? 1 : 0;
            dst.csd.mq.flags.mirror_z = (axis & MQOBJECT_MIRROR_AXIS_Z) ? 1 : 0;
        }
    }

    dst.flags.visible = obj->GetVisible();
    dst.flags.has_points = 1;
    dst.flags.has_uv = 1;
    dst.flags.has_counts = 1;
    dst.flags.has_indices = 1;

    // copy vertices
    int npoints = obj->GetVertexCount();
    dst.points.resize(npoints);
    obj->GetVertexArray((MQPoint*)dst.points.data());
    if (m_scale_factor != 1.0f) {
        mu::Scale(dst.points.data(), m_scale_factor, dst.points.size());
    }

    // copy counts
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

    // copy indices & uv
    dst.indices.resize(nindices);
    dst.uv.resize(nindices);
    auto *indices = dst.indices.data();
    auto *uv = dst.uv.data();
    for (int fi = 0; fi < nfaces; ++fi) {
        int c = dst.counts[fi];
        if (c >= 3 /*&& obj->GetFaceVisible(fi)*/) {
            obj->GetFacePointArray(fi, indices);
            obj->GetFaceCoordinateArray(fi, (MQCoordinate*)uv);
            indices += c;
            uv += c;
        }
    }

    // remove line and points
    dst.counts.erase(
        std::remove_if(dst.counts.begin(), dst.counts.end(), [](int c) { return c < 3; }),
        dst.counts.end());
}
