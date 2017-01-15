#include "pch.h"
#include "Sync.h"

#define MaxNameBuffer 256

Sync::Sync()
{
}

ms::ClientSettings& Sync::getClientSettings() { return m_settings; }
float& Sync::getScaleFactor() { return m_scale_factor; }
bool& Sync::getAutoSync() { return m_auto_sync; }

void Sync::send(MQDocument doc, bool force)
{
    if (!force && !m_auto_sync) { return; }

    int nobj = doc->GetObjectCount();
    while ((int)m_data.size() < nobj) {
        m_data.emplace_back(new ms::MeshData());
    }

    // gather object data
    concurrency::parallel_for(0, nobj, [this, doc](int i) {
        auto& data = *m_data[i];
        gather(doc, doc->GetObject(i), data);

        ms::Client client(m_settings);
        client.send(data);
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

        ms::Client client(m_settings);
        client.send(send_data.data(), (int)send_data.size());
    }
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
    gd.flags.apply_transform = 1;
    gd.flags.bake_skin = 1;
    gd.scale = 1.0f / m_scale_factor;

    auto ret = client.send(gd);
    for (auto& data : ret) {
        auto& mdata = static_cast<ms::MeshData&>(*data);
        if (auto obj = findObject(doc, mdata.path)) {
            doc->DeleteObject(doc->GetObjectIndex(obj));
        }
        auto obj = createObject(mdata);
        doc->AddObject(obj);
    }
}

MQObject Sync::findObject(MQDocument doc, const std::string & name)
{
    int nobj = doc->GetObjectCount();
    for (int i = 0; i < nobj; ++i) {
        auto obj = doc->GetObject(i);
        if (!obj) { continue; }

        char tmp[MaxNameBuffer];
        obj->GetName(tmp, sizeof(tmp));
        if (strcmp(tmp, name.c_str()) == 0) {
            return obj;
        }
    }
    return nullptr;
}

MQObject Sync::createObject(const ms::MeshData& data)
{
    auto ret = MQ_CreateObject();
    ret->SetName(data.path.c_str());
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
    dst.csd.type = ms::ClientSpecificData::Type::Metasequia;
    dst.csd.mq.smooth_angle = obj->GetSmoothAngle();

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
        if (c >= 3) {
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
