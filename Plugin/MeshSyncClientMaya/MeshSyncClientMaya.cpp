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

static void OnDagChange(MDagMessage::DagMessage msg, MDagPath &child, MDagPath &parent, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onSceneUpdated();
}

static void OnTimeChange(MTime& time, void* _this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onTimeChange(time);
}

static void OnNodeRemoved(MObject& node, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onNodeRemoved(node);
}


static void OnTransformUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    if ((msg & MNodeMessage::kAttributeEval) != 0) { return; }
    reinterpret_cast<MeshSyncClientMaya*>(_this)->notifyUpdateTransform(plug.node());
}

static void OnCameraUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    if ((msg & MNodeMessage::kAttributeEval) != 0) { return; }
    reinterpret_cast<MeshSyncClientMaya*>(_this)->notifyUpdateCamera(plug.node());
}

static void OnLightUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    if ((msg & MNodeMessage::kAttributeEval) != 0) { return; }
    reinterpret_cast<MeshSyncClientMaya*>(_this)->notifyUpdateLight(plug.node());
}

static void OnMeshUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
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
    , m_iplugin(obj, "Unity Technologies", "20170915")
{
#define Body(CmdType) m_iplugin.registerCommand(CmdType::name(), CmdType::create, CmdType::createSyntax);
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
        m_future_send.wait_for(std::chrono::milliseconds(m_settings.timeout_ms));
    }
}

void MeshSyncClientMaya::registerGlobalCallbacks()
{
    MStatus stat;
    m_cids_global.push_back(MTimerMessage::addTimerCallback(0.03f, OnIdle, this, &stat));
    m_cids_global.push_back(MEventMessage::addEventCallback("SelectionChanged", OnSelectionChanged, this, &stat));
    m_cids_global.push_back(MDagMessage::addAllDagChangesCallback(OnDagChange, this, &stat));
    m_cids_global.push_back(MDGMessage::addTimeChangeCallback(OnTimeChange, this));
    m_cids_global.push_back(MDGMessage::addNodeRemovedCallback(OnNodeRemoved, kDefaultNodeType, this));

    // shut up warning about blendshape
    MGlobal::executeCommand("cycleCheck -e off");
}

void MeshSyncClientMaya::registerNodeCallbacks(TargetScope scope)
{
    MStatus stat;
    if (scope == TargetScope::Selection) {
        MSelectionList list;
        MGlobal::getActiveSelectionList(list);
        for (uint32_t i = 0; i < list.length(); i++) {
            MObject node;
            list.getDependNode(i, node);
            registerNodeCallback(node);
        }
    }
    else {
        //  meshes
        Enumerate(MFn::kMesh, [this](MObject& node) {
            registerNodeCallback(GetTransform(node));
        });

        // cameras
        Enumerate(MFn::kCamera, [this](MObject& node) {
            registerNodeCallback(GetTransform(node));
        });

        // lights
        Enumerate(MFn::kLight, [this](MObject& node) {
            registerNodeCallback(GetTransform(node));
        });

        // joints
        Enumerate(MFn::kJoint, [this](MObject& node) {
            registerNodeCallback(node);
        });
    }
}

bool MeshSyncClientMaya::registerNodeCallback(MObject node, bool leaf)
{
    if (node.isNull() || node.hasFn(MFn::kWorld))
        return false;

    bool handled = false;
    auto shape = GetShape(node);
    if (!shape.isNull()) {
        if (shape.hasFn(MFn::kMesh)) {
            MFnMesh fn(shape);
            if (!fn.isIntermediateObject()) {
                mscTrace("track mesh %s\n", GetName(node).c_str());
                auto& rec = findOrAddRecord(node);
                if (!rec.cid_trans) rec.cid_trans = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
                if (!rec.cid_shape) rec.cid_shape = MNodeMessage::addAttributeChangedCallback(shape, OnMeshUpdated, this);
            }
            handled = true;
        }
        else if (shape.hasFn(MFn::kCamera)) {
            MFnCamera fn(shape);
            if (!fn.isIntermediateObject()) {
                mscTrace("track camera %s\n", GetName(node).c_str());
                auto& rec = findOrAddRecord(node);
                if (!rec.cid_trans) rec.cid_trans = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
                if (!rec.cid_shape) rec.cid_shape = MNodeMessage::addAttributeChangedCallback(shape, OnCameraUpdated, this);
            }
            handled = true;
        }
        else if (shape.hasFn(MFn::kLight)) {
            MFnLight fn(shape);
            if (!fn.isIntermediateObject()) {
                mscTrace("track light %s\n", GetName(node).c_str());
                auto& rec = findOrAddRecord(node);
                if (!rec.cid_trans) rec.cid_trans = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
                if (!rec.cid_shape) rec.cid_shape = MNodeMessage::addAttributeChangedCallback(shape, OnLightUpdated, this);
            }
            handled = true;
        }
    }

    if (!handled) {
        if (node.hasFn(MFn::kJoint)) {
            mscTrace("track joint %s\n", GetName(node).c_str());
            auto& rec = findOrAddRecord(node);
            if (!rec.cid_trans)rec.cid_trans = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
            handled = true;
        }
    }

    if (!handled && !leaf && node.hasFn(MFn::kTransform)) {
        mscTrace("track transform %s\n", GetName(node).c_str());
        auto& rec = findOrAddRecord(node);
        if (!rec.cid_trans)rec.cid_trans = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
        handled = true;
    }
    if (handled) {
        registerNodeCallback(GetParent(node), false);
    }
    return handled;
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
    for (auto& rec : m_records) {
        if (rec.second.cid_trans) {
            MMessage::removeCallback(rec.second.cid_trans);
            rec.second.cid_trans = 0;
        }
        if (rec.second.cid_shape) {
            MMessage::removeCallback(rec.second.cid_shape);
            rec.second.cid_shape = 0;
        }
    }
}


int MeshSyncClientMaya::getMaterialID(MUuid uid)
{
    std::unique_lock<std::mutex> lock(m_mutex_extract_mesh);

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

MeshSyncClientMaya::ObjectRecord& MeshSyncClientMaya::findOrAddRecord(MObject node)
{
    auto& record = m_records[(void*&)node];
    if (record.path.empty()) {
        record = ObjectRecord { node, GetName(node), GetPath(node), };
        mscTrace("MeshSyncClientMaya::addRecord(): %s\n", record.path.c_str());
    }
    return record;
}

bool MeshSyncClientMaya::addToDirtyList(MObject node)
{
    if (std::find(m_dirty_objects.begin(), m_dirty_objects.end(), node) == m_dirty_objects.end()) {
        m_dirty_objects.push_back(node);
        return true;
    }
    return false;
}

void MeshSyncClientMaya::notifyUpdateTransform(MObject node, bool force)
{
    if ((force || m_settings.auto_sync) && node.hasFn(MFn::kTransform)) {
        findOrAddRecord(node).dirty_transform = true;
        addToDirtyList(node);
    }
}

void MeshSyncClientMaya::notifyUpdateCamera(MObject shape, bool force)
{
    if ((force || m_settings.auto_sync) && m_settings.sync_cameras && shape.hasFn(MFn::kCamera)) {
        auto node = GetTransform(shape);
        findOrAddRecord(node).dirty_shape = true;
        addToDirtyList(node);
    }
}

void MeshSyncClientMaya::notifyUpdateLight(MObject shape, bool force)
{
    if ((force || m_settings.auto_sync) && m_settings.sync_lights && shape.hasFn(MFn::kLight)) {
        auto node = GetTransform(shape);
        findOrAddRecord(node).dirty_shape = true;
        addToDirtyList(node);
    }
}

void MeshSyncClientMaya::notifyUpdateMesh(MObject shape, bool force)
{
    if ((force || m_settings.auto_sync) && m_settings.sync_meshes && shape.hasFn(MFn::kMesh)) {
        auto node = GetTransform(shape);
        findOrAddRecord(node).dirty_shape = true;
        addToDirtyList(node);
    }
}

void MeshSyncClientMaya::update()
{
    if (m_scene_updated) {
        m_scene_updated = false;
        mscTrace("MeshSyncClientMaya::update(): handling scene update\n");

        registerNodeCallbacks();
        if (m_settings.auto_sync) {
            m_pending_send_scene = true;
        }
    }

    if (m_pending_send_scene) {
        if (sendScene()) {
            mscTrace("MeshSyncClientMaya::update(): handling send scene\n");
        }
    }
    else {
        if (sendMarkedObjects()) {
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
    mscTrace("MeshSyncClientMaya::onSceneUpdated()\n");
}

void MeshSyncClientMaya::onTimeChange(MTime & time)
{
    sendScene();
    mscTrace("MeshSyncClientMaya::onTimeChange()\n");
}

void MeshSyncClientMaya::onNodeRemoved(MObject & node)
{
    if (node.hasFn(MFn::kTransform)) {
        auto it = m_records.find((void*&)node);
        if (it != m_records.end()) {
            mscTrace("MeshSyncClientMaya::onNodeRemoved(): %s\n", it->second.path.c_str());
            m_deleted.push_back(it->second.path);
            m_records.erase(it);
        }
    }
}

bool MeshSyncClientMaya::sendScene(TargetScope scope)
{
    if (isAsyncSendInProgress()) {
        m_pending_send_scene = true;
        return false;
    }
    m_pending_send_scene = false;

    // clear dirty list and add all objects
    m_dirty_objects.clear();

    if(scope == TargetScope::All) {
        // meshes
        Enumerate(MFn::kMesh, [this](MObject& shape) {
            MFnMesh fn(shape);
            if (!fn.isIntermediateObject())
                notifyUpdateMesh(shape, true);
        });

        // cameras
        Enumerate(MFn::kCamera, [this](MObject& shape) {
            MFnCamera fn(shape);
            if (!fn.isIntermediateObject())
                notifyUpdateCamera(shape, true);
        });

        // lights
        Enumerate(MFn::kLight, [this](MObject& shape) {
            MFnLight fn(shape);
            if (!fn.isIntermediateObject())
                notifyUpdateLight(shape, true);
        });

        // joints
        Enumerate(MFn::kJoint, [this](MObject& node) {
            notifyUpdateTransform(node, true);
        });
    }
    else if (scope == TargetScope::Selection) {
        // selected meshes
        MStatus stat;
        MSelectionList list;
        MGlobal::getActiveSelectionList(list);

        for (uint32_t i = 0; i < list.length(); i++) {
            MObject node;
            list.getDependNode(i, node);
            auto path = GetDagPath(node);
            if (path.extendToShape() == MStatus::kSuccess) {
                auto shape = path.node();
                if (shape.hasFn(MFn::kMesh)) {
                    MFnMesh fn(shape);
                    if (!fn.isIntermediateObject()) {
                        notifyUpdateMesh(node, true);
                    }
                }
            }
        }
    }

    return sendMarkedObjects();
}

bool MeshSyncClientMaya::sendMarkedObjects()
{
    if (isAsyncSendInProgress() || (m_dirty_objects.empty() && m_deleted.empty())) {
        return false;
    }

    extractSceneData();

    // note: this must be index based because joints maybe added to m_dirty_objects in this loop.
    for (size_t i = 0; i < m_dirty_objects.size();++i) {
        auto node = m_dirty_objects[i];
        auto shape = GetShape(node);
        auto& record = findOrAddRecord(node);

        // check rename / re-parenting
        auto path = GetPath(node);
        if (record.path != path) {
            mscTrace("rename %s -> %s\n", record.path.c_str(), path.c_str());
            m_deleted.push_back(record.path);
            record.name = GetName(node);
            record.path = path;
            record.dirty_transform = true;
            record.dirty_shape = true;
        }

        if (record.dirty_shape && shape.hasFn(MFn::kMesh)) {
            auto mesh = ms::MeshPtr(new ms::Mesh());
            if (extractMeshData(*mesh, node)) {
                m_client_meshes.emplace_back(mesh);
            }
        }
        else if (record.dirty_shape && shape.hasFn(MFn::kCamera)) {
            auto dst = ms::CameraPtr(new ms::Camera());
            if (extractCameraData(*dst, node)) {
                m_client_cameras.emplace_back(dst);
            }
        }
        else if (record.dirty_shape && shape.hasFn(MFn::kLight)) {
            auto dst = ms::LightPtr(new ms::Light());
            if (extractLightData(*dst, node)) {
                m_client_lights.emplace_back(dst);
            }
        }
        else if(record.dirty_shape || record.dirty_transform) {
            auto dst = ms::TransformPtr(new ms::Transform());
            if (extractTransformData(*dst, node)) {
                m_client_transforms.emplace_back(dst);
            }
        }
        record.dirty_shape = record.dirty_transform = false;
    }
    m_dirty_objects.clear();

    kickAsyncSend();
    return true;
}

bool MeshSyncClientMaya::importScene()
{
    waitAsyncSend();

    ms::Client client(m_settings.client_settings);
    ms::GetMessage gd;
    gd.flags.get_transform = 1;
    gd.flags.get_indices = 1;
    gd.flags.get_points = 1;
    gd.flags.get_uv0 = 1;
    gd.flags.get_uv1 = 1;
    gd.flags.get_material_ids= 1;
    gd.scene_settings.handedness = ms::Handedness::Right;
    gd.scene_settings.scale_factor = 1.0f / m_settings.scale_factor;
    gd.refine_settings.flags.bake_skin = m_settings.bake_skin;
    gd.refine_settings.flags.bake_cloth = m_settings.bake_cloth;

    auto ret = client.send(gd);
    if (!ret) {
        return false;
    }


    // todo: create mesh objects

    return true;
}

void MeshSyncClientMaya::kickAsyncSend()
{
    m_future_send = std::async(std::launch::async, [this]() {
        mscTrace("MeshSyncClientMaya::kickAsyncSend(): kicked\n");
        ms::Client client(m_settings.client_settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::Right;
        scene_settings.scale_factor = m_settings.scale_factor;

        // notify scene begin
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneBegin;
            client.send(fence);
        }

        // send scene data (except meshes)
        {
            ms::SetMessage set;
            set.scene.settings  = scene_settings;
            set.scene.transforms= m_client_transforms;
            set.scene.cameras   = m_client_cameras;
            set.scene.lights    = m_client_lights;
            set.scene.materials = m_client_materials;
            client.send(set);

            m_client_transforms.clear();
            m_client_cameras.clear();
            m_client_lights.clear();
            m_client_materials.clear();
        }

        // send meshes one by one to Unity can respond quickly
        for(auto& mesh : m_client_meshes) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.meshes = { mesh };
            client.send(set);
        };
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
