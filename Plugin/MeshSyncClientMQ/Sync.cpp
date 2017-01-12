#include "pch.h"
#include "Sync.h"

Sync::Sync()
{
}

void Sync::setClientSettings(const ms::ClientSettings& v)
{
    m_settings = v;
}

void Sync::sync(MQDocument doc)
{
    int nobj = doc->GetObjectCount();
    while ((int)m_data.size() < nobj) {
        m_data.emplace_back(new ms::MeshData());
    }
    m_fences.resize(nobj);

    // gather object data
    concurrency::parallel_for(0, nobj, [doc, this](int i) {
        auto& data = *m_data[i];
        gather(doc, doc->GetObject(i), data);

        ms::Client client(m_settings);
        client.send(data);
    });

    // detect deleted objects
    m_prev_objects.swap(m_current_objects);
    m_current_objects.resize(nobj);
    for (int i = 0; i < nobj; ++i) {
        m_current_objects[i] = m_data[i]->obj_path;
    }
    std::sort(m_current_objects.begin(), m_current_objects.end());

    std::vector<ms::DeleteData> del_data;
    for (auto& n : m_prev_objects) {
        if (std::lower_bound(m_current_objects.begin(), m_current_objects.end(), n) == m_current_objects.end()) {
            ms::DeleteData data;
            data.obj_path = n;
            del_data.push_back(data);
        }
    }
    if (!del_data.empty()) {
        std::vector<ms::EventData*> send_data(del_data.size());
        for (size_t i = 0; i < del_data.size(); ++i) { send_data[i] = &del_data[i]; }

        ms::Client client(m_settings);
        client.send(send_data.data(), (int)send_data.size());

    }
}

inline void BuildPath(MQDocument doc, MQObject obj, std::string& path)
{
    if (auto parent = doc->GetParentObject(obj)) {
        BuildPath(doc, parent, path);
    }
    char name[256];
    obj->GetName(name, sizeof(name));
    path += "/";
    path += name;
}

void Sync::gather(MQDocument doc, MQObject obj, ms::MeshData& dst)
{
    dst.obj_path.clear();
    BuildPath(doc, obj, dst.obj_path);
    dst.smooth_angle = obj->GetSmoothAngle();

    // copy vertices
    int npoints = obj->GetVertexCount();
    dst.points.resize(npoints);
    obj->GetVertexArray((MQPoint*)dst.points.data());

    // copy counts
    int nfaces = obj->GetFaceCount();
    int nindices = 0;
    dst.counts.resize(nfaces);
    for (int fi = 0; fi < nfaces; ++fi) {
        int c = obj->GetFacePointCount(fi);
        dst.counts[fi] = c;
        nindices += c;
    }

    // copy indices & uv
    dst.indices.resize(nindices);
    dst.uv.resize(nindices);
    auto *indices = dst.indices.data();
    auto *uv = dst.uv.data();
    for (int fi = 0; fi < nfaces; ++fi) {
        int c = dst.counts[fi];
        obj->GetFacePointArray(fi, indices);
        obj->GetFaceCoordinateArray(fi, (MQCoordinate*)uv);
        indices += c;
        uv += c;
    }
}
