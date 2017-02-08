#include "pch.h"
#include "MeshSyncClientMaya.h"

#ifdef _WIN32
#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "OpenMaya.lib")
#endif


inline bool IsVisible(MFnDagNode& dag)
{
    if (dag.isIntermediateObject()) {
        return false;
    }

    auto vis = dag.findPlug("visibility");
    bool visible = false;
    vis.getValue(visible);
    return visible;
}


std::unique_ptr<MeshSyncClientMaya> g_plugin;

static void OnSceneUpdate(void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onSceneUpdate();
}
static void* OnSyncCommand()
{
    if (g_plugin) {
        g_plugin->sendMeshes();
    }
    return nullptr;
}



#define msMayaCmdSync "UnityMeshSync_Sync"


MeshSyncClientMaya::MeshSyncClientMaya(MObject obj)
    : m_obj(obj)
    , m_iplugin(obj, "Unity Technologies", "0.8.0")
{
    m_iplugin.registerCommand(msMayaCmdSync, OnSyncCommand);
    m_cid_sceneupdate = MSceneMessage::addCallback(MSceneMessage::kSceneUpdate, OnSceneUpdate, this);
}

MeshSyncClientMaya::~MeshSyncClientMaya()
{
    m_future_send.wait_for(std::chrono::milliseconds(5000));
    m_iplugin.deregisterCommand(msMayaCmdSync);
    MSceneMessage::removeCallback(m_cid_sceneupdate);
}

void MeshSyncClientMaya::onSceneUpdate()
{
    if (m_auto_sync) {
        sendMeshes();
    }
}

void MeshSyncClientMaya::sendMeshes()
{
    if (isAsyncSendInProgress()) {
        m_pending_send_meshes = true;
        return;
    }
    m_pending_send_meshes = false;

    // gather mesh data
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
        m_client_meshes.resize(num_meshes);
    }

    // kick async send
    m_future_send = std::async(std::launch::async, [this]() {
        ms::Client client(m_client_settings);

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
        concurrency::parallel_for_each(m_client_meshes.begin(), m_client_meshes.end(), [&scene_settings, &client](ms::MeshPtr& mesh) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.meshes = { mesh };
            client.send(set);
        });

        // detect deleted objects and send delete message
        {
            for (auto& e : m_exist_record) {
                e.second = false;
            }
            for (auto& mesh : m_client_meshes) {
                if (!mesh->path.empty()) {
                    m_exist_record[mesh->path] = true;
                }
            }

            ms::DeleteMessage del;
            for (auto i = m_exist_record.begin(); i != m_exist_record.end(); ) {
                if (!i->second) {
                    int id = 0;
                    //ExtractID(i->first.c_str(), id);
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

bool MeshSyncClientMaya::isAsyncSendInProgress() const
{
    if (m_future_send.valid()) {
        return m_future_send.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
    }
    return false;
}



void MeshSyncClientMaya::gatherMeshData(ms::Mesh& dst, MObject src)
{
    MFnMesh src_mesh(src);
    MFnTransform src_trs(src);

    MDagPath path;
    MFloatPointArray points;
    MFloatVectorArray normals;
    MStringArray uvsets;
    MFloatArray u;
    MFloatArray v;

    src_mesh.getPath(path);
    src_mesh.getPoints(points);
    src_mesh.getNormals(normals);
    src_mesh.getUVSetNames(uvsets);


    dst.clear();
    dst.path = path.fullPathName().asChar();

    dst.flags.visible = IsVisible(src_mesh);
    if (!dst.flags.visible) { return; }

    dst.flags.has_points = 1;
    dst.flags.has_normals = 1;
    dst.flags.has_counts = 1;
    dst.flags.has_indices = 1;
    //dst.flags.has_materialIDs = 1;
    dst.flags.has_refine_settings = 1;
    dst.refine_settings.flags.gen_tangents = 1;

    {
        auto len = points.length();
        for (uint32_t i = 0; i < len; ++i) {
            dst.points.push_back((const mu::float3&)points[i]);
        }
    }

    {
        MItMeshPolygon it_poly(src);
        while (!it_poly.isDone()) {
            int count = it_poly.polygonVertexCount();
            dst.counts.push_back(count);
            for (int i = 0; i < count; ++i) {
                int iv = it_poly.vertexIndex(i);
                int in = it_poly.normalIndex(i);

                dst.indices.push_back(iv);
                dst.normals.push_back((const mu::float3&)normals[in]);
            }
            it_poly.next();
        }
    }

    if (uvsets.length() > 0 && src_mesh.numUVs(uvsets[0]) > 0) {
        dst.flags.has_uv = 1;
        src_mesh.getUVs(u, v, &uvsets[0]);

        MItMeshPolygon it_poly(src);
        while (!it_poly.isDone()) {
            int count = it_poly.polygonVertexCount();
            for (int i = 0; i < count; ++i) {
                int iu;
                it_poly.getUVIndex(i, iu, &uvsets[0]);

                dst.uv.push_back(mu::float2{ u[iu], v[iu] });
            }
            it_poly.next();
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
