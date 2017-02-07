#include "pch.h"
#include "MeshSyncClientMaya.h"

std::unique_ptr<MeshSyncClientMaya> g_plugin;

static void OnSceneUpdate(void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onSceneUpdate();
}


MeshSyncClientMaya::MeshSyncClientMaya(MObject obj)
    : m_obj(obj)
    , m_iplugin(obj, "Unity Technologies", "0.8.0")
{
    //m_iplugin.registerCommand(kmoduleLogicCmdName, moduleLogicCmd::creator);
    m_cid_sceneupdate = MSceneMessage::addCallback(MSceneMessage::kSceneUpdate, OnSceneUpdate, this);
}

MeshSyncClientMaya::~MeshSyncClientMaya()
{
    //m_iplugin.deregisterCommand(kmoduleLogicCmdName);
    MSceneMessage::removeCallback(m_cid_sceneupdate);
}

void MeshSyncClientMaya::onSceneUpdate()
{
    if (isAsyncSendInProgress()) {
        m_pending_send_meshes = true;
        return;
    }

    gatherMeshData();
}

bool MeshSyncClientMaya::isAsyncSendInProgress() const
{
    if (m_future_send.valid()) {
        return m_future_send.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
    }
    return false;
}

void MeshSyncClientMaya::gatherMeshData()
{
    int num_meshes = 0;

    MItDag it(MItDag::kDepthFirst, MFn::kMesh);
    while (!it.isDone()) {
        MFnMesh fn(it.item());
        if (!fn.isIntermediateObject()) {
            if (m_client_meshes.size() <= num_meshes) {
                m_client_meshes.emplace_back(new ms::Mesh());
            }
            auto& cmesh = *m_client_meshes[num_meshes];
            cmesh.clear();
            gatherMeshData(cmesh, fn.object());

            ++num_meshes;
        }

        it.next();
    }
}

void MeshSyncClientMaya::gatherMeshData(ms::Mesh& dst, MObject src)
{
    MFnMesh src_mesh(src);
    MFnTransform src_trs(src);

    MPointArray points;
    MFloatVectorArray normals;
    MStringArray uvsets;
    MFloatArray u;
    MFloatArray v;

    src_mesh.getPoints(points);
    src_mesh.getNormals(normals);

    src_mesh.getUVSetNames(uvsets);
    if (uvsets.length() > 0 && src_mesh.numUVs(uvsets[0]) > 0) {
        src_mesh.getUVs(u, v, &uvsets[0]);
    }

    MItMeshPolygon it_poly(src);
    while (!it_poly.isDone()) {
        int count = it_poly.polygonVertexCount();
        for (int i = 0; i < count; ++i) {
            int iv = it_poly.vertexIndex(i);
            int in = it_poly.normalIndex(i);
            int iu;
            it_poly.getUVIndex(i, iu, &uvsets[0]);

        }
    }
}




MStatus initializePlugin(MObject obj)
{
    g_plugin.reset(new MeshSyncClientMaya(obj));
    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
    g_plugin.reset();
    return MS::kSuccess;
}
