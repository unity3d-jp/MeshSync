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

    concurrency::parallel_for(0, nobj, [doc, this](int i) {
        auto& data = *m_data[i];
        gather(doc->GetObject(i), data);

        ms::Client client(m_settings);
        client.send(data);
    });
}

void Sync::gather(MQObject obj, ms::MeshData& dst)
{
    char name[1024];
    obj->GetName(name, sizeof(name));
    dst.obj_path = name;

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
