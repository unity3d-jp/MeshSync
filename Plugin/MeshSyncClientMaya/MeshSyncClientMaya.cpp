#include "pch.h"
#include "MayaUtils.h"
#include "MeshSyncClientMaya.h"
#include "Commands.h"

#ifdef _WIN32
#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "OpenMaya.lib")
#pragma comment(lib, "OpenMayaAnim.lib")
#endif



static void OnIdle(float elapsedTime, float lastTime, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->update();
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
    registerNodeCallbacks();
}

MeshSyncClientMaya::~MeshSyncClientMaya()
{
    waitAsyncSend();
    removeNodeCallbacks();
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
    m_cids_global.push_back(MTimerMessage::addTimerCallback(0.03f, OnIdle, this, &stat));
    m_cids_global.push_back(MEventMessage::addEventCallback("SelectionChanged", OnSelectionChanged, this, &stat));
    m_cids_global.push_back(MDagMessage::addAllDagChangesCallback(OnChangeDag, this, &stat));
}

void MeshSyncClientMaya::registerNodeCallbacks(TargetScope scope)
{
    removeNodeCallbacks();

    MStatus stat;
    if (scope == TargetScope::Selection) {
        MSelectionList list;
        MGlobal::getActiveSelectionList(list);

        for (uint32_t i = 0; i < list.length(); i++)
        {
            MObject node;
            MDagPath path;
            list.getDependNode(i, node);
            list.getDagPath(i, path);

            if (node.hasFn(MFn::kJoint)) {
                mscTrace("tracking transform %s\n", path.fullPathName().asChar());
                m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(node, OnChangeTransform, this, &stat));
            }
            else {
                auto path_to_shape = path;
                if (path_to_shape.extendToShape() == MS::kSuccess && path_to_shape.node().hasFn(MFn::kMesh)) {
                    mscTrace("tracking mesh %s\n", path.fullPathName().asChar());
                    m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(node, OnChangeTransform, this, &stat));
                    m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(path_to_shape.node(), OnChangeMesh, this, &stat));
                }
            }
        }
    }
    else {
        MItDag it(MItDag::kDepthFirst, MFn::kMesh);
        while (!it.isDone()) {
            auto shape = it.item();
            MFnMesh fn(shape);
            if (!fn.isIntermediateObject()) {
                auto trans = GetTransform(shape);
                mscTrace("tracking mesh %s\n", MDagPath::getAPathTo(trans).fullPathName().asChar());
                m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(trans, OnChangeTransform, this, &stat));
                m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(shape, OnChangeMesh, this, &stat));
            }
            it.next();
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

void MeshSyncClientMaya::removeNodeCallbacks()
{
    for (auto& cid : m_cids_node) {
        MMessage::removeCallback(cid);
    }
    m_cids_node.clear();
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

void MeshSyncClientMaya::notifyUpdateTransform(MObject node)
{
    if (m_auto_sync && node.hasFn(MFn::kTransform)) {
        if (std::find(m_mtransforms.begin(), m_mtransforms.end(), node) == m_mtransforms.end()) {
            mscTrace("MeshSyncClientMaya::notifyUpdateTransform()\n");
            m_mtransforms.push_back(node);
        }
    }
}

void MeshSyncClientMaya::notifyUpdateMesh(MObject shape)
{
    if (m_auto_sync && shape.hasFn(MFn::kMesh)) {
        auto node = GetTransform(shape);
        if (std::find(m_mmeshes.begin(), m_mmeshes.end(), node) == m_mmeshes.end()) {
            mscTrace("MeshSyncClientMaya::notifyUpdateMesh()\n");
            m_mmeshes.push_back(node);
        }
    }
}

void MeshSyncClientMaya::update()
{
    if (m_scene_updated) {
        m_scene_updated = false;
        mscTrace("MeshSyncClientMaya::update(): handling scene update\n");

        registerNodeCallbacks();

        // detect deleted objects
        {
            for (auto& e : m_exist_record) {
                e.second = false;
            }

            {
                MItDag it(MItDag::kDepthFirst, MFn::kMesh);
                while (!it.isDone()) {
                    m_exist_record[GetPath(GetTransform(it.item()))] = true;
                    it.next();
                }
            }
            {
                MItDag it(MItDag::kDepthFirst, MFn::kJoint);
                while (!it.isDone()) {
                    m_exist_record[GetPath(it.item())] = true;
                    it.next();
                }
            }

            for (auto i = m_exist_record.begin(); i != m_exist_record.end(); ) {
                if (!i->second) {
                    mscTrace("MeshSyncClientMaya::update(): detected %s is deleted\n", i->first.c_str());
                    m_deleted.push_back(i->first);
                    m_exist_record.erase(i++);
                }
                else {
                    ++i;
                }
            }
        }

        if (m_auto_sync) {
            m_pending_send_scene = true;
        }
    }

    if (m_pending_send_scene) {
        if (sendScene()) {
            mscTrace("MeshSyncClientMaya::update(): handling send scene\n");
        }
    }
    else {
        if (sendUpdatedObjects()) {
            mscTrace("MeshSyncClientMaya::update(): handling send updated objects\n");
        }
    }
}

void MeshSyncClientMaya::onSelectionChanged()
{
}

void MeshSyncClientMaya::onSceneUpdated()
{
    m_scene_updated = true;
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

bool MeshSyncClientMaya::sendUpdatedObjects()
{
    if (isAsyncSendInProgress() || (m_mtransforms.empty() && m_mmeshes.empty())) {
        return false;
    }

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
    return true;
}

bool MeshSyncClientMaya::sendScene(TargetScope scope)
{
    if (isAsyncSendInProgress()) {
        m_pending_send_scene = true;
        return false;
    }
    m_pending_send_scene = false;
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
    return true;
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
    dst.transform.position.assign(&pos[0]);
    dst.transform.rotation.assign(&rot[0]);
    dst.transform.scale.assign(scale);
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
    dst.flags.apply_trs = 1;
    dst.refine_settings.flags.gen_tangents = 1;
    dst.refine_settings.flags.swap_faces = 1;
    dst.refine_settings.flags.generate_weights4 = 1;

    MFnMesh fn_mesh = FindMesh(src);
    if (fn_mesh.object().isNull()) { return; }
    MFnSkinCluster fn_skin = FindSkinCluster(fn_mesh.object());
    MFnMesh fn_src_mesh = fn_skin.object().isNull() ? fn_mesh.object() : FindOrigMesh(src);

    // get points
    {
        MFloatPointArray points;
        fn_src_mesh.getPoints(points);

        auto len = points.length();
        dst.points.resize(len);
        for (uint32_t i = 0; i < len; ++i) {
            dst.points[i] = (const mu::float3&)points[i];
        }

        // todo: apply tweak
        //if (!fn_skin.object().isNull()) {
        //    auto plug_pnts = fn_mesh.findPlug("pnts");
        //    auto pnts_len = std::min<uint32_t>(len, plug_pnts.numElements());
        //    for (uint32_t i = 0; i < pnts_len; ++i) {
        //        MPlug p3 = plug_pnts.elementByPhysicalIndex(i);
        //        mu::float3 v;
        //        p3.child(0).getValue(v.x);
        //        p3.child(1).getValue(v.y);
        //        p3.child(2).getValue(v.z);
        //        dst.points[i] += v;
        //    }
        //}
    }

    // get normals and faces
    {
        MFloatVectorArray normals;
        fn_src_mesh.getNormals(normals);

        MItMeshPolygon it_poly(fn_src_mesh.object());
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
        fn_src_mesh.getUVSetNames(uvsets);

        if (uvsets.length() > 0 && fn_src_mesh.numUVs(uvsets[0]) > 0) {
            dst.flags.has_uv = 1;

            MFloatArray u;
            MFloatArray v;
            fn_src_mesh.getUVs(u, v, &uvsets[0]);

            MItMeshPolygon it_poly(fn_src_mesh.object());
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


    // get face material id
    {
        std::vector<int> mids;
        MObjectArray shaders;
        MIntArray indices;
        fn_mesh.getConnectedShaders(0, shaders, indices);
        mids.resize(shaders.length(), -1);
        for (uint32_t si = 0; si < shaders.length(); si++) {
            MItDependencyGraph it(shaders[si], MFn::kLambert, MItDependencyGraph::kUpstream);
            if (!it.isDone()) {
                MFnLambertShader lambert(it.currentItem());
                mids[si] = getMaterialID(lambert.uuid());
            }
        }

        if (mids.size() == 1) {
            dst.materialIDs.resize(indices.length(), mids[0]);
        }
        else {
            dst.materialIDs.resize(indices.length());
            for (uint32_t i = 0; i < indices.length(); ++i) {
                dst.materialIDs[i] = mids[indices[i]];
            }
        }
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

            MObject matrix_obj;
            auto ijoint = fn_skin.indexForInfluenceObject(joint_paths[ij], nullptr);
            plug_bindprematrix.elementByLogicalIndex(ijoint).getValue(matrix_obj);
            MMatrix bindpose = MFnMatrixData(matrix_obj).matrix();
            dst.bindposes[ij].assign(bindpose[0]);
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
        mscTrace("MeshSyncClientMaya::kickAsyncSend(): kicked\n");
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
        mscTrace("MeshSyncClientMaya::kickAsyncSend(): done\n");
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
