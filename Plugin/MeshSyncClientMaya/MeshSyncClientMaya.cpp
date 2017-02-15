#include "pch.h"
#include "MeshSyncClientMaya.h"
#include "Commands.h"

#ifdef _WIN32
#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "OpenMaya.lib")
#pragma comment(lib, "OpenMayaAnim.lib")
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


static void OnSelectionChanged(void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onSelectionChanged();
}

static void OnChangeDag(MDagMessage::DagMessage msg, MDagPath &child, MDagPath &parent, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->notifyDAGChanged();
}

static void OnChangeTransform(MObject& obj, MDagMessage::MatrixModifiedFlags& flags, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->notifyUpdateTransform(obj);
}

static void OnChangeMesh(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    if ((msg & MNodeMessage::kAttributeEval) != 0) { return; }
    reinterpret_cast<MeshSyncClientMaya*>(_this)->notifyUpdateMesh(plug.node());
}


static std::unique_ptr<MeshSyncClientMaya> g_plugin;


MeshSyncClientMaya& MeshSyncClientMaya::getInstance()
{
    return *g_plugin;
}

MeshSyncClientMaya::MeshSyncClientMaya(MObject obj)
    : m_obj(obj)
    , m_iplugin(obj, "Unity Technologies", "0.8.0")
{
#define Body(CmdType) m_iplugin.registerCommand(CmdType::name(), CmdType::create);
    EachCommand(Body)
#undef Body

    registerGlobalCallbacks();
}

MeshSyncClientMaya::~MeshSyncClientMaya()
{
    waitAsyncSend();
    removeSelectionCallbacks();
    removeGlobalCallbacks();
#define Body(CmdType) m_iplugin.deregisterCommand(CmdType::name());
    EachCommand(Body)
#undef Body
}

bool MeshSyncClientMaya::isAsyncSendInProgress() const
{
    if (m_future_send.valid()) {
        return m_future_send.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout;
    }
    return false;
}

void MeshSyncClientMaya::waitAsyncSend()
{
    if (m_future_send.valid()) {
        m_future_send.wait_for(std::chrono::milliseconds(m_timeout_ms));
    }
}

void MeshSyncClientMaya::registerGlobalCallbacks()
{
    MStatus stat;
    m_cids_global.push_back(MEventMessage::addEventCallback("SelectionChanged", OnSelectionChanged, this, &stat));
    m_cids_global.push_back(MDagMessage::addAllDagChangesCallback(OnChangeDag, this, &stat));
}

void MeshSyncClientMaya::registerSelectionCallbacks()
{
    MStatus stat;
    MSelectionList list;
    MGlobal::getActiveSelectionList(list);

    for (uint32_t i = 0; i < list.length(); i++)
    {
        MObject node;
        list.getDependNode(i, node);

        if (node.hasFn(MFn::kTransform)) {
            MDagPath path = MDagPath::getAPathTo(node);
            mscTrace("tracking %s\n", path.fullPathName().asChar());
            m_cids_selection.push_back(MDagMessage::addWorldMatrixModifiedCallback(path, OnChangeTransform, this, &stat));
        }

        MDagPath path;
        if (MDagPath::getAPathTo(node, path) == MS::kSuccess) {
            if (path.extendToShape() == MS::kSuccess) {
                auto shape = path.node();
                mscTrace("tracking %s\n", path.fullPathName().asChar());
                m_cids_selection.push_back(MNodeMessage::addAttributeChangedCallback(shape, OnChangeMesh, this, &stat));
            }
        }
    }
}

void MeshSyncClientMaya::removeGlobalCallbacks()
{
    for (auto& cid : m_cids_global) {
        MMessage::removeCallback(cid);
    }
    m_cids_global.clear();
}

void MeshSyncClientMaya::removeSelectionCallbacks()
{
    for (auto& cid : m_cids_selection) {
        MMessage::removeCallback(cid);
    }
    m_cids_selection.clear();
}


int MeshSyncClientMaya::getMaterialID(MUuid uid)
{
    auto i = std::find(m_material_id_table.begin(), m_material_id_table.end(), uid);
    if (i != m_material_id_table.end()) {
        return (int)std::distance(m_material_id_table.begin(), i);
    }
    else {
        mscTrace("MeshSyncClientMaya::getMaterialID(): new ID\n");
        int id = (int)m_material_id_table.size();
        m_material_id_table.push_back(uid);
        return id;
    }
}

int MeshSyncClientMaya::getObjectID(MUuid uid)
{
    auto i = std::find(m_object_id_table.begin(), m_object_id_table.end(), uid);
    if (i != m_object_id_table.end()) {
        return (int)std::distance(m_object_id_table.begin(), i);
    }
    else {
        mscTrace("MeshSyncClientMaya::getObjectID() new ID\n");
        int id = (int)m_object_id_table.size();
        m_object_id_table.push_back(uid);
        return id;
    }
}

void MeshSyncClientMaya::notifyDAGChanged()
{
}

void MeshSyncClientMaya::notifyUpdateTransform(MObject obj)
{
    mscTrace("MeshSyncClientMaya::notifyUpdateTransform()\n");
    if (m_auto_sync) {
        m_mtransforms.push_back(obj);
    }
}

void MeshSyncClientMaya::notifyUpdateMesh(MObject obj)
{
    mscTrace("MeshSyncClientMaya::notifyUpdateMesh()\n");
    if (m_auto_sync) {
        m_mmeshes.push_back(obj);
    }
}

void MeshSyncClientMaya::onIdle()
{
    //mscTrace("MeshSyncClientMaya::onIdle()\n");
    sendUpdatedObjects();
}

void MeshSyncClientMaya::onSelectionChanged()
{
    mscTrace("MeshSyncClientMaya::onSelectionChanged()\n");
    removeSelectionCallbacks();
    registerSelectionCallbacks();
}

void MeshSyncClientMaya::onSceneUpdate()
{
    mscTrace("MeshSyncClientMaya::onSceneUpdate()\n");
    if (m_auto_sync) {
        sendScene();
    }
}

void MeshSyncClientMaya::setServerAddress(const char * v)
{
    m_client_settings.server = v;
}

void MeshSyncClientMaya::setServerPort(uint16_t v)
{
    m_client_settings.port = v;
}

void MeshSyncClientMaya::setAutoSync(bool v)
{
    m_auto_sync = v;
}

void MeshSyncClientMaya::sendUpdatedObjects()
{
    if (!m_mtransforms.empty() || !m_mmeshes.empty()) {
        mscTrace("MeshSyncClientMaya::sendUpdatedObjects()\n");

        auto pless = [](MObject& a, MObject& b) { return (void*&)a < (void*&)b; };
        auto pequal = [](MObject& a, MObject& b) { return (void*&)a == (void*&)b; };

        std::sort(m_mtransforms.begin(), m_mtransforms.end(), pless);
        m_mtransforms.erase(std::unique(m_mtransforms.begin(), m_mtransforms.end(), pequal), m_mtransforms.end());
        std::sort(m_mmeshes.begin(), m_mmeshes.end(), pless);
        m_mmeshes.erase(std::unique(m_mmeshes.begin(), m_mmeshes.end(), pequal), m_mmeshes.end());

        // gather material data
        extractAllMaterialData();

        for (auto& mtrans : m_mtransforms) {
            if (mtrans.hasFn(MFn::kJoint) || mtrans.hasFn(MFn::kMesh)) {
                auto trans = new ms::Transform();
                m_client_transforms.emplace_back(trans);
                extractTransformData(*trans, mtrans);
            }
        }
        for (auto& mmesh : m_mmeshes) {
            auto mesh = new ms::Mesh();
            m_client_meshes.emplace_back(mesh);
            extractMeshData(*mesh, mmesh);
        }

        m_mtransforms.clear();
        m_mmeshes.clear();

        kickAsyncSend();
    }
}

void MeshSyncClientMaya::sendScene(TargetScope scope)
{
    if (isAsyncSendInProgress()) {
        m_pending_send_meshes = true;
        return;
    }
    m_pending_send_meshes = false;

    mscTrace("MeshSyncClientMaya::sendScene()\n");
    m_mtransforms.clear();
    m_mmeshes.clear();

    // gather material data
    extractAllMaterialData();

    // gather mesh data
    if(scope == TargetScope::All) {
        // all meshes
        MItDag it(MItDag::kDepthFirst, MFn::kMesh);
        while (!it.isDone()) {
            MFnMesh fn(it.item());
            if (!fn.isIntermediateObject()) {
                auto mesh = new ms::Mesh();
                m_client_meshes.emplace_back(mesh);
                extractMeshData(*mesh, fn.object());
            }

            it.next();
        }
    }
    else if (scope == TargetScope::Selection) {
        // selected meshes
        MStatus stat;
        MSelectionList list;
        MGlobal::getActiveSelectionList(list);

        for (uint32_t i = 0; i < list.length(); i++) {
            MObject node;
            list.getDependNode(i, node);

            MDagPath path;
            if (MDagPath::getAPathTo(node, path) == MS::kSuccess) {
                if (path.extendToShape() == MS::kSuccess) {
                    auto shape = path.node();
                    auto mesh = new ms::Mesh();
                    m_client_meshes.emplace_back(mesh);
                    extractTransformData(*mesh, node);
                    extractMeshData(*mesh, path.node());
                }
            }
        }
    }

    kickAsyncSend();
}

bool MeshSyncClientMaya::importScene()
{
    waitAsyncSend();

    ms::Client client(m_client_settings);
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

    // todo: create materials

    // todo: create mesh objects

    return true;
}

void MeshSyncClientMaya::extractAllMaterialData()
{
    MItDependencyNodes it(MFn::kLambert);
    while (!it.isDone()) {
        MFnLambertShader fn(it.item());

        ms::Material tmp;
        tmp.name = fn.name().asChar();
        tmp.color = (const mu::float4&)fn.color();
        tmp.id = getMaterialID(fn.uuid());
        m_materials.push_back(tmp);

        it.next();
    }
}

void MeshSyncClientMaya::extractTransformData(ms::Transform& dst, MObject src)
{
    MFnTransform src_trs(src);

    MStatus stat;
    MVector pos;
    MQuaternion rot;
    double scale[3];

    pos = src_trs.getTranslation(MSpace::kObject, &stat);
    stat = src_trs.getRotation(rot, MSpace::kObject);
    stat = src_trs.getScale(scale);
    dst.transform.position = { (float)pos[0], (float)pos[1], (float)pos[2] };
    dst.transform.rotation = { (float)rot[0], (float)rot[1], (float)rot[2], (float)rot[3] };
    dst.transform.scale = { (float)scale[0], (float)scale[1], (float)scale[2] };
}

void MeshSyncClientMaya::extractMeshData(ms::Mesh& dst, MObject src)
{
    MFnMesh src_mesh(src);

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
    dst.refine_settings.flags.swap_handedness = 1;
    dst.refine_settings.flags.generate_weights4 = 1;

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


    // get material data
    {
        MObjectArray shaders;
        MIntArray indices;
        src_mesh.getConnectedShaders(0, shaders, indices);
        for (uint32_t si = 0; si < shaders.length(); si++) {
            MFnDependencyNode shader_group(shaders[si]);
            MPlug shader_plug = shader_group.findPlug("surfaceShader");
            MPlugArray connections;
            shader_plug.connectedTo(connections, true, false);
            for (uint32_t ci = 0; ci < connections.length(); ci++) {
                if (connections[ci].node().hasFn(MFn::kLambert)) {
                    MFnLambertShader lambert(connections[ci].node());
                    int id = getMaterialID(lambert.uuid());
                }
            }
        }
    }

    // get skinning data
    {
        MPlug in_mesh;
        MPlugArray dependencies;
        MFnDependencyNode node(src);

        in_mesh = node.findPlug("inMesh");
        in_mesh.connectedTo(dependencies, true, false);

        auto num_dependencies = dependencies.length();
        for (uint32_t idp = 0; idp < num_dependencies; idp++) {
            MFnDependencyNode node(dependencies[idp].node());
            if (node.typeName() == "skinCluster") {
                MFnSkinCluster skin_cluster(dependencies[idp].node());
                MDagPathArray joint_paths;

                // get bindposes
                MPlug plug_bindprematrix = skin_cluster.findPlug("bindPreMatrix");
                auto num_joints = skin_cluster.influenceObjects(joint_paths);
                dst.bindposes.resize(num_joints);
                for (uint32_t ij = 0; ij < num_joints; ij++) {
                    auto bone = new ms::Transform();
                    m_client_transforms.emplace_back(bone);
                    bone->path = joint_paths[ij].partialPathName().asChar();

                    auto ijoint = skin_cluster.indexForInfluenceObject(joint_paths[ij], nullptr);
                    MPlug plug_matrix = plug_bindprematrix.elementByLogicalIndex(ijoint);

                    MObject matrix_obj;
                    plug_matrix.getValue(matrix_obj);
                    MFnMatrixData matrix_data(matrix_obj);
                    MMatrix bindpose = matrix_data.matrix();
                    auto& dst_bp = dst.bindposes[ij];
                    for (int ir = 0; ir < 4; ++ir) {
                        dst_bp[ir] = { (float)bindpose[ir][0], (float)bindpose[ir][1], (float)bindpose[ir][2], (float)bindpose[ir][3] };
                    }
                }

                // get weights
                auto num_meshes = skin_cluster.numOutputConnections();
                for (uint32_t im = 0; im < num_meshes; im++) {
                    auto index = skin_cluster.indexForOutputConnection(im);

                    MDagPath mesh_path;
                    skin_cluster.getPathAtIndex(index, mesh_path);

                    MItGeometry it_geom(mesh_path);
                    while (!it_geom.isDone()) {
                        MObject component = it_geom.component();
                        MFloatArray weights;
                        uint32_t influence_count;
                        skin_cluster.getWeights(mesh_path, component, weights, influence_count);

                        for (uint32_t ij = 0; ij < influence_count; ij++) {
                            dst.bone_weights.push_back(weights[ij]);
                        }

                        it_geom.next();
                    }
                }
            }
        }
    }
}

void MeshSyncClientMaya::kickAsyncSend()
{
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

        m_materials.clear();
        m_client_transforms.clear();
        m_client_meshes.clear();
    });
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
