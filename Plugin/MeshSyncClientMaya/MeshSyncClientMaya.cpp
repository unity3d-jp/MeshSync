#include "pch.h"
#include "MeshSyncClientMaya.h"
#include "Commands.h"

#ifdef _WIN32
#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "OpenMaya.lib")
#pragma comment(lib, "OpenMayaAnim.lib")
#endif

template<class Body>
static void EachChild(MObject node, const Body& body)
{
    MFnDagNode fn = node;
    auto num_children = fn.childCount();
    for (uint32_t i = 0; i < num_children; ++i) {
        body(fn.child(i));
    }
}

static bool IsVisible(MObject node)
{
    MFnDagNode dag = node;
    auto vis = dag.findPlug("visibility");
    bool visible = false;
    vis.getValue(visible);
    return visible;
}

static std::string GetPath(MDagPath path)
{
    std::string ret = path.fullPathName().asChar();
    std::replace(ret.begin(), ret.end(), '|', '/');
    return ret;
}
static std::string GetPath(MObject node)
{
    return GetPath(MDagPath::getAPathTo(node));
}

static MObject GetTransform(MDagPath path)
{
    return path.transform();
}
static MObject GetTransform(MObject node)
{
    return GetTransform(MDagPath::getAPathTo(node));
}

static MObject FindSkinCluster(MObject node)
{
    MItDependencyGraph it(node, MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
    if (!it.isDone()) {
        return it.currentItem();
    }
    return MObject();
}

static MObject FindMesh(MObject node)
{
    MObject ret;
    EachChild(node, [&](MObject child) {
        if (child.hasFn(MFn::kMesh)) {
            MFnMesh fn = child;
            if (!fn.isIntermediateObject()) {
                ret = child;
            }
        }
    });
    return ret;
}

static MObject FindOrigMesh(MObject node)
{
    MObject ret;
    EachChild(node, [&](MObject child) {
        if (child.hasFn(MFn::kMesh)) {
            MFnMesh fn = child;
            if (ret.isNull() || fn.isIntermediateObject()) {
                ret = child;
            }
        }
    });
    return ret;
}



static void OnIdle(void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onIdle();
}

static void OnSelectionChanged(void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onSelectionChanged();
}

static void OnChangeDag(MDagMessage::DagMessage msg, MDagPath &child, MDagPath &parent, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onSceneUpdated();
}

static void OnChangeTransform(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    if ((msg & MNodeMessage::kAttributeEval) != 0) { return; }
    reinterpret_cast<MeshSyncClientMaya*>(_this)->notifyUpdateTransform(plug.node());
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
    m_cids_global.push_back(MEventMessage::addEventCallback("idle", OnIdle, this, &stat));
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
        MDagPath path;
        list.getDependNode(i, node);
        list.getDagPath(i, path);

        if(node.hasFn(MFn::kJoint)) {
            mscTrace("tracking transform %s\n", path.fullPathName().asChar());
            m_cids_selection.push_back(MNodeMessage::addAttributeChangedCallback(node, OnChangeTransform, this, &stat));
        }
        else {
            auto path_to_shape = path;
            if (path_to_shape.extendToShape() == MS::kSuccess && path_to_shape.node().hasFn(MFn::kMesh)) {
                mscTrace("tracking mesh %s\n", path.fullPathName().asChar());
                m_cids_selection.push_back(MNodeMessage::addAttributeChangedCallback(node, OnChangeTransform, this, &stat));
                m_cids_selection.push_back(MNodeMessage::addAttributeChangedCallback(path_to_shape.node(), OnChangeMesh, this, &stat));
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

void MeshSyncClientMaya::notifyUpdateTransform(MObject obj)
{
    mscTrace("MeshSyncClientMaya::notifyUpdateTransform()\n");
    if (m_auto_sync) {
        m_mtransforms.push_back(obj);
    }
}

void MeshSyncClientMaya::notifyUpdateMesh(MObject node)
{
    mscTrace("MeshSyncClientMaya::notifyUpdateMesh()\n");
    if (m_auto_sync) {
        m_mmeshes.push_back(GetTransform(node));
    }
}

void MeshSyncClientMaya::onIdle()
{
    //mscTrace("MeshSyncClientMaya::onIdle()\n");
    if (m_pending_send_meshes) {
        sendScene();
    }
    else {
        sendUpdatedObjects();
    }
}

void MeshSyncClientMaya::onSelectionChanged()
{
    mscTrace("MeshSyncClientMaya::onSelectionChanged()\n");
    removeSelectionCallbacks();
    registerSelectionCallbacks();
}

void MeshSyncClientMaya::onSceneUpdated()
{
    mscTrace("MeshSyncClientMaya::onSceneUpdate()\n");


    // detect deleted objects
    {
        for (auto& e : m_exist_record) {
            e.second = false;
        }

        {
            MItDag it(MItDag::kDepthFirst, MFn::kMesh);
            while (!it.isDone()) {
                auto node = it.item();
                MFnMesh fn(node);
                if (!fn.isIntermediateObject()) {
                    m_exist_record[GetPath(GetTransform(node))] = true;
                }

                it.next();
            }
        }

        for (auto i = m_exist_record.begin(); i != m_exist_record.end(); ) {
            if (!i->second) {
                m_deleted.push_back(i->first);
                m_exist_record.erase(i++);
            }
            else {
                ++i;
            }
        }
    }

    if (m_auto_sync) {
        m_pending_send_meshes = true;
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

        for (auto& mtrans : m_mtransforms) {
            auto trans = new ms::Transform();
            m_client_transforms.emplace_back(trans);
            extractTransformData(*trans, mtrans);
        }

        if (!m_mmeshes.empty()) {
            extractAllMaterialData();
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
            auto node = it.item();
            MFnMesh fn(node);
            if (!fn.isIntermediateObject()) {
                auto mesh = new ms::Mesh();
                m_client_meshes.emplace_back(mesh);
                extractMeshData(*mesh, GetTransform(node));
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
                    extractMeshData(*mesh, node);
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

    dst.path = GetPath(src);

    pos = src_trs.getTranslation(MSpace::kTransform, &stat);
    stat = src_trs.getRotation(rot, MSpace::kTransform);
    stat = src_trs.getScale(scale);
    dst.transform.position = { (float)pos[0], (float)pos[1], (float)pos[2] };
    dst.transform.rotation = { (float)rot[0], (float)rot[1], (float)rot[2], (float)rot[3] };
    dst.transform.scale = { (float)scale[0], (float)scale[1], (float)scale[2] };
}

void MeshSyncClientMaya::extractMeshData(ms::Mesh& dst, MObject src)
{
    dst.clear();

    extractTransformData(dst, src);

    dst.flags.visible = IsVisible(src);
    if (!dst.flags.visible) { return; }

    dst.flags.has_points = 1;
    dst.flags.has_normals = 1;
    dst.flags.has_counts = 1;
    dst.flags.has_indices = 1;
    dst.flags.has_materialIDs = 1;
    dst.flags.has_refine_settings = 1;
    dst.refine_settings.flags.gen_tangents = 1;
    dst.refine_settings.flags.swap_faces = 1;
    dst.refine_settings.flags.generate_weights4 = 1;

    MFnMesh fn_mesh = FindMesh(src);
    MFnMesh fn_mesh_orig = FindOrigMesh(src);
    MFnSkinCluster fn_skin = FindSkinCluster(fn_mesh.object());
    if (fn_mesh.object().isNull()) { return; }

    // get points
    {
        MFloatPointArray points;
        fn_mesh_orig.getPoints(points);

        auto len = points.length();
        for (uint32_t i = 0; i < len; ++i) {
            dst.points.push_back((const mu::float3&)points[i]);
        }
    }

    // get normals and faces
    {
        MFloatVectorArray normals;
        fn_mesh_orig.getNormals(normals);

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

    // get uv
    {
        MStringArray uvsets;
        fn_mesh_orig.getUVSetNames(uvsets);

        if (uvsets.length() > 0 && fn_mesh_orig.numUVs(uvsets[0]) > 0) {
            dst.flags.has_uv = 1;

            MFloatArray u;
            MFloatArray v;
            fn_mesh_orig.getUVs(u, v, &uvsets[0]);

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


    // get material data
    {
        int mid = -1;
        MObjectArray shaders;
        MIntArray indices;
        fn_mesh.getConnectedShaders(0, shaders, indices);
        for (uint32_t si = 0; si < shaders.length(); si++) {
            MFnDependencyNode shader_group(shaders[si]);
            MPlug shader_plug = shader_group.findPlug("surfaceShader");
            MPlugArray connections;
            shader_plug.connectedTo(connections, true, false);
            for (uint32_t ci = 0; ci < connections.length(); ci++) {
                if (connections[ci].node().hasFn(MFn::kLambert)) {
                    MFnLambertShader lambert(connections[ci].node());
                    mid = getMaterialID(lambert.uuid());
                }
            }
        }
        dst.materialIDs.resize(dst.counts.size(), mid);
    }

    // get skinning data
    if(!fn_skin.object().isNull()) {

        // get bone data
        MPlug plug_bindprematrix = fn_skin.findPlug("bindPreMatrix");
        MDagPathArray joint_paths;
        auto num_joints = fn_skin.influenceObjects(joint_paths);

        dst.flags.has_bones = 1;
        dst.bones_per_vertex = num_joints;
        dst.bones.resize(num_joints);
        dst.bindposes.resize(num_joints);

        for (uint32_t ij = 0; ij < num_joints; ij++) {
            auto bone = new ms::Transform();
            m_client_transforms.emplace_back(bone);
            extractTransformData(*bone, joint_paths[ij].node());
            dst.bones[ij] = bone->path;

            auto ijoint = fn_skin.indexForInfluenceObject(joint_paths[ij], nullptr);
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
        auto num_meshes = fn_skin.numOutputConnections();
        for (uint32_t im = 0; im < num_meshes; im++) {
            auto index = fn_skin.indexForOutputConnection(im);

            MDagPath mesh_path;
            fn_skin.getPathAtIndex(index, mesh_path);

            MItGeometry it_geom(mesh_path);
            while (!it_geom.isDone()) {
                MObject component = it_geom.component();
                MFloatArray weights;
                uint32_t influence_count;
                fn_skin.getWeights(mesh_path, component, weights, influence_count);

                for (uint32_t ij = 0; ij < influence_count; ij++) {
                    dst.bone_indices.push_back(ij);
                    dst.bone_weights.push_back(weights[ij]);
                }

                it_geom.next();
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

        // send materials & transforms
        {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.materials = m_materials;
            set.scene.transforms = m_client_transforms;
            client.send(set);

            m_materials.clear();
            m_client_transforms.clear();
        }

        // send meshes one by one to Unity can respond quickly
        concurrency::parallel_for_each(m_client_meshes.begin(), m_client_meshes.end(), [&scene_settings, &client](ms::MeshPtr& mesh) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.meshes = { mesh };
            client.send(set);
        });
        m_client_meshes.clear();

        // send delete message
        if(!m_deleted.empty()) {
            ms::DeleteMessage del;
            del.targets.resize(m_deleted.size());
            for (size_t i = 0; i < m_deleted.size(); ++i) {
                del.targets[i].path = m_deleted[i];
            }
            client.send(del);
            m_deleted.clear();
        }

        // notify scene end
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneEnd;
            client.send(fence);
        }
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
