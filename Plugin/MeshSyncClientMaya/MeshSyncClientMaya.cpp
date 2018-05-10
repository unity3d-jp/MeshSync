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
    if (msg == MNodeMessage::kAttributeEval) { return; }
    reinterpret_cast<MeshSyncClientMaya*>(_this)->notifyUpdateTransform(plug.node());
}

static void OnCameraUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->notifyUpdateCamera(plug.node());
}

static void OnLightUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
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

bool MeshSyncClientMaya::isSending() const
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

void MeshSyncClientMaya::registerNodeCallbacks()
{
    // cameras
    Enumerate(MFn::kCamera, [this](MObject& shape) {
        registerNodeCallback(GetTransform(shape));
    });

    // lights
    Enumerate(MFn::kLight, [this](MObject& shape) {
        registerNodeCallback(GetTransform(shape));
    });

    // joints
    Enumerate(MFn::kJoint, [this](MObject& node) {
        registerNodeCallback(node);
    });

    //  meshes
    Enumerate(MFn::kMesh, [this](MObject& shape) {
        registerNodeCallback(GetTransform(shape));
    });
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
    auto it = std::find(m_material_id_table.begin(), m_material_id_table.end(), uid);
    return it != m_material_id_table.end() ? (int)std::distance(m_material_id_table.begin(), it) : -1;
}

ms::TransformPtr MeshSyncClientMaya::exportObject(MObject node, bool force)
{
    if (node.isNull())
        return nullptr;

    auto& rec = findOrAddRecord(node);
    if (rec.added)
        return nullptr;

    rec.added = true;
    auto shape = GetShape(node);

    // check rename / re-parenting
    auto path = GetPath(node);
    if (rec.path != path) {
        mscTrace("rename %s -> %s\n", rec.path.c_str(), path.c_str());
        m_deleted.push_back(rec.path);
        rec.name = GetName(node);
        rec.path = path;
        rec.dirty_transform = true;
        rec.dirty_shape = true;
    }

    ms::TransformPtr ret;
    if (m_settings.sync_meshes && shape.hasFn(MFn::kMesh)) {
        exportObject(GetParent(node), true);
        auto dst = ms::MeshPtr(new ms::Mesh());
        extractMeshData(*dst, node);
        m_client_meshes.emplace_back(dst);
        ret = dst;
    }
    else if (m_settings.sync_cameras &&shape.hasFn(MFn::kCamera)) {
        exportObject(GetParent(node), true);
        auto dst = ms::CameraPtr(new ms::Camera());
        extractCameraData(*dst, node);
        m_client_cameras.emplace_back(dst);
        ret = dst;
    }
    else if (m_settings.sync_lights &&shape.hasFn(MFn::kLight)) {
        exportObject(GetParent(node), true);
        auto dst = ms::LightPtr(new ms::Light());
        extractLightData(*dst, node);
        m_client_lights.emplace_back(dst);
        ret = dst;
    }
    else if((m_settings.sync_bones && shape.hasFn(MFn::kJoint)) || force) {
        exportObject(GetParent(node), true);
        auto dst = ms::TransformPtr(new ms::Transform());
        extractTransformData(*dst, node);
        m_client_transforms.emplace_back(dst);
        ret = dst;
    }

    if (ret) {
        ret->index = rec.index;
    }
    return ret;
}

MeshSyncClientMaya::ObjectRecord& MeshSyncClientMaya::findOrAddRecord(MObject node)
{
    auto& rec = m_records[(void*&)node];
    if (rec.path.empty()) {
        rec.node = node;
        rec.name = GetName(node);
        rec.path = GetPath(node);
        rec.index = ++m_index_seed;
        mscTrace("MeshSyncClientMaya::addRecord(): %s\n", rec.path.c_str());
    }
    return rec;
}

const MeshSyncClientMaya::ObjectRecord* MeshSyncClientMaya::findRecord(MObject node)
{
    auto it = m_records.find((void*&)node);
    return it == m_records.end() ? nullptr : &it->second;
}

void MeshSyncClientMaya::notifyUpdateTransform(MObject node)
{
    findOrAddRecord(node).dirty_transform = true;
}

void MeshSyncClientMaya::notifyUpdateCamera(MObject shape)
{
    auto node = GetTransform(shape);
    findOrAddRecord(node).dirty_shape = true;
}

void MeshSyncClientMaya::notifyUpdateLight(MObject shape)
{
    auto node = GetTransform(shape);
    findOrAddRecord(node).dirty_shape = true;
}

void MeshSyncClientMaya::notifyUpdateMesh(MObject shape)
{
    auto node = GetTransform(shape);
    findOrAddRecord(node).dirty_shape = true;
}

bool MeshSyncClientMaya::send(SendScope scope)
{
    if (isSending()) {
        m_pending_send_scene = scope;
        return false;
    }
    m_pending_send_scene = SendScope::None;

    int num_exported = 0;
    if (scope == SendScope::All) {
        auto exportShape = [this, &num_exported](MObject& shape) {
            if (exportObject(GetTransform(shape), false))
                ++num_exported;
        };
        auto exportNode = [this, &num_exported](MObject& node) {
            if (exportObject(node, false))
                ++num_exported;
        };

        Enumerate(MFn::kCamera, exportShape);
        Enumerate(MFn::kLight, exportShape);
        Enumerate(MFn::kJoint, exportNode);
        Enumerate(MFn::kMesh, exportShape);
    }
    else if (scope == SendScope::Updated) {
        for (auto& kvp : m_records) {
            auto& rec = kvp.second;
            if (rec.dirty_transform || rec.dirty_shape) {
                if (exportObject(rec.node, false))
                    ++num_exported;
            }
        }
    }
    else if (scope == SendScope::Selected) {
        MSelectionList list;
        MGlobal::getActiveSelectionList(list);
        for (uint32_t i = 0; i < list.length(); i++) {
            MObject node;
            list.getDependNode(i, node);
            if (exportObject(node, false))
                ++num_exported;
        }
    }
    else if (scope == SendScope::Animations) {
        num_exported = exportAnimations();
    }

    if (num_exported > 0 || !m_deleted.empty()) {
        kickAsyncSend();
        return true;
    }
    else {
        return false;
    }
}

void MeshSyncClientMaya::update()
{
    if (m_scene_updated) {
        m_scene_updated = false;
        mscTrace("MeshSyncClientMaya::update(): handling scene update\n");

        registerNodeCallbacks();
        if (m_settings.auto_sync) {
            m_pending_send_scene = SendScope::All;
        }
    }

    if (m_pending_send_scene != SendScope::None) {
        if (send(m_pending_send_scene)) {
            mscTrace("MeshSyncClientMaya::update(): handling send scene\n");
        }
    }
    else if (m_settings.auto_sync) {
        if (send(SendScope::Updated)) {
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
    m_scene_updated = true;
    update();
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

void MeshSyncClientMaya::kickAsyncSend()
{
    if (!m_extract_tasks.empty()) {
        mu::parallel_for_each(m_extract_tasks.begin(), m_extract_tasks.end(), [](task_t& task) {
            task();
        });
        m_extract_tasks.clear();
    }

    m_material_id_table.clear();

    for (auto& kvp : m_records) {
        auto& rec = kvp.second;
        rec.added = rec.dirty_shape = rec.dirty_transform = false;
    }

    float to_meter = 1.0f;
    {
        MDistance dist;
        dist.setValue(1.0f);
        to_meter = (float)dist.asMeters();
    }

    m_future_send = std::async(std::launch::async, [this, to_meter]() {
        mscTrace("MeshSyncClientMaya::kickAsyncSend(): kicked\n");
        ms::Client client(m_settings.client_settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::Right;
        scene_settings.scale_factor = m_settings.scale_factor / to_meter;

        // notify scene begin
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneBegin;
            client.send(fence);
        }

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

        // send scene data
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

        // send animations and constraints
        if (!m_client_animations.empty() || !m_client_constraints.empty()) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.animations = m_client_animations;
            set.scene.constraints = m_client_constraints;
            client.send(set);

            m_client_animations.clear();
            m_client_constraints.clear();
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

bool MeshSyncClientMaya::import()
{
    waitAsyncSend();

    ms::Client client(m_settings.client_settings);
    ms::GetMessage gd;
    gd.flags.get_transform = 1;
    gd.flags.get_indices = 1;
    gd.flags.get_points = 1;
    gd.flags.get_uv0 = 1;
    gd.flags.get_uv1 = 1;
    gd.flags.get_material_ids = 1;
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
