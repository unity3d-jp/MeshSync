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


void MeshSyncClientMaya::ObjectRecord::clear()
{
    if (dagpaths.length() > 0) {
        dagpaths_prev = dagpaths;
        dagpaths.clear();
    }
    dirty_shape = dirty_transform = false;
}
bool MeshSyncClientMaya::ObjectRecord::isAdded(const MDagPath& dpath) const
{
    return Find(dagpaths, dpath);
}
bool MeshSyncClientMaya::ObjectRecord::wasAdded(const MDagPath& dpath) const
{
    return Find(dagpaths_prev, dpath);
}
void MeshSyncClientMaya::ObjectRecord::addPath(const MDagPath& dpath)
{
    dagpaths.append(dpath);
}

void MeshSyncClientMaya::ExtractRecord::addTask(const task_t & task)
{
    tasks.push_back(task);
}

void MeshSyncClientMaya::ExtractRecord::processTasks()
{
    for (auto& t : tasks)
        t();
    tasks.clear();
}

void MeshSyncClientMaya::ObjectRecord::dbgPrint() const
{
    mscTrace("path:\n");
    EachPath(node, [](const MDagPath& dpath) {
        mscTrace("  %s\n", GetPath(dpath).c_str());
    });
    mscTrace("added:\n");
    Each(dagpaths, [](const MDagPath& dpath) {
        mscTrace("  %s\n", GetPath(dpath).c_str());
    });
    mscTrace("prev added:\n");
    Each(dagpaths_prev, [](const MDagPath& dpath) {
        mscTrace("  %s\n", GetPath(dpath).c_str());
    });
}



MeshSyncClientMaya& MeshSyncClientMaya::getInstance()
{
    return *g_plugin;
}

MeshSyncClientMaya::MeshSyncClientMaya(MObject obj)
    : m_obj(obj)
    , m_iplugin(obj, msVendor, msReleaseDateStr)
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
    EnumerateNode(MFn::kCamera, [this](MObject& shape) {
        registerNodeCallback(GetTransform(shape));
    });

    // lights
    EnumerateNode(MFn::kLight, [this](MObject& shape) {
        registerNodeCallback(GetTransform(shape));
    });

    // joints
    EnumerateNode(MFn::kJoint, [this](MObject& node) {
        registerNodeCallback(node);
    });

    //  meshes
    EnumerateNode(MFn::kMesh, [this](MObject& shape) {
        registerNodeCallback(GetTransform(shape));
    });
}

bool MeshSyncClientMaya::registerNodeCallback(MObject& node, bool leaf)
{
    if (node.isNull() || node.hasFn(MFn::kWorld))
        return false;

    bool handled = false;
    auto shape = GetShape(node);
    if (!shape.isNull()) {
        if (shape.hasFn(MFn::kMesh)) {
            MFnMesh fn(shape);
            if (!fn.isIntermediateObject()) {
                auto& rec = findOrAddRecord(node);
                if (!rec.cid_trans) rec.cid_trans = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
                if (!rec.cid_shape) rec.cid_shape = MNodeMessage::addAttributeChangedCallback(shape, OnMeshUpdated, this);
            }
            handled = true;
        }
        else if (shape.hasFn(MFn::kCamera)) {
            MFnCamera fn(shape);
            if (!fn.isIntermediateObject()) {
                auto& rec = findOrAddRecord(node);
                if (!rec.cid_trans) rec.cid_trans = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
                if (!rec.cid_shape) rec.cid_shape = MNodeMessage::addAttributeChangedCallback(shape, OnCameraUpdated, this);
            }
            handled = true;
        }
        else if (shape.hasFn(MFn::kLight)) {
            MFnLight fn(shape);
            if (!fn.isIntermediateObject()) {
                auto& rec = findOrAddRecord(node);
                if (!rec.cid_trans) rec.cid_trans = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
                if (!rec.cid_shape) rec.cid_shape = MNodeMessage::addAttributeChangedCallback(shape, OnLightUpdated, this);
            }
            handled = true;
        }
    }

    if (!handled) {
        if (node.hasFn(MFn::kJoint)) {
            auto& rec = findOrAddRecord(node);
            if (!rec.cid_trans) rec.cid_trans = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
            handled = true;
        }
    }

    if (!handled && !leaf && node.hasFn(MFn::kTransform)) {
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
    for (auto& rec : m_object_records) {
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

bool MeshSyncClientMaya::exportObject(MDagPath dagpath, bool force)
{
    auto node = dagpath.transform();
    if (node.isNull())
        return false;

    auto& rec = findOrAddRecord(node);
    if (rec.isAdded(dagpath))
        return false;

    auto& shape = rec.shape;

    ms::TransformPtr ret;
    if (m_settings.sync_meshes && shape.hasFn(MFn::kMesh)) {
        exportObject(GetParent(dagpath), true);
        auto dst = ms::Mesh::create();
        extractMeshData(*dst, node, shape);
        m_client_meshes.emplace_back(dst);
        ret = dst;
    }
    else if (m_settings.sync_cameras &&shape.hasFn(MFn::kCamera)) {
        exportObject(GetParent(dagpath), true);
        auto dst = ms::Camera::create();
        extractCameraData(*dst, node, shape);
        m_client_objects.emplace_back(dst);
        ret = dst;
    }
    else if (m_settings.sync_lights &&shape.hasFn(MFn::kLight)) {
        exportObject(GetParent(dagpath), true);
        auto dst = ms::Light::create();
        extractLightData(*dst, node, shape);
        m_client_objects.emplace_back(dst);
        ret = dst;
    }
    else if ((m_settings.sync_bones && shape.hasFn(MFn::kJoint)) || force) {
        exportObject(GetParent(dagpath), true);
        auto dst = ms::Transform::create();
        extractTransformData(*dst, node);
        m_client_objects.emplace_back(dst);
        ret = dst;
    }

    if (ret) {
        ret->path = GetPath(dagpath);
        ret->index = rec.index;
        rec.addPath(dagpath);
        return true;
    }
    else {
        return false;
    }
}

MeshSyncClientMaya::ObjectRecord& MeshSyncClientMaya::findOrAddRecord(const MObject& node)
{
    auto& rec = m_object_records[(void*&)node];
    if (rec.node.isNull()) {
        rec.node = node;
        rec.shape = GetShape(node);
        rec.index = ++m_index_seed;
    }
    return rec;
}

const MeshSyncClientMaya::ObjectRecord* MeshSyncClientMaya::findRecord(const MObject& node)
{
    auto it = m_object_records.find((void*&)node);
    return it == m_object_records.end() ? nullptr : &it->second;
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

bool MeshSyncClientMaya::sendScene(SendScope scope)
{
    if (isSending()) {
        m_pending_send_scene = scope;
        return false;
    }
    m_pending_send_scene = SendScope::None;

    int num_exported = 0;
    if (scope == SendScope::All) {
        auto exportNode = [this, &num_exported](MDagPath& node) {
            if (exportObject(node, false))
                ++num_exported;
        };
        auto exportShape = [this, &num_exported](MDagPath& shape) {
            shape.pop(); // shape to transform
            if (exportObject(shape, false))
                ++num_exported;
        };

        EnumeratePath(MFn::kJoint, exportNode);
        EnumeratePath(MFn::kCamera, exportShape);
        EnumeratePath(MFn::kLight, exportShape);
        EnumeratePath(MFn::kMesh, exportShape);
    }
    else if (scope == SendScope::Updated) {
        for (auto& kvp : m_object_records) {
            auto& rec = kvp.second;
            if (rec.dirty_transform || rec.dirty_shape) {
                if (!rec.shape.isNull()) {
                    EachPath(rec.shape, [&](MDagPath& shape) {
                        shape.pop(); // shape to transform
                        if (exportObject(shape, false))
                            ++num_exported;
                    });
                }
                else {
                    EachPath(rec.node, [&](const MDagPath& node) {
                        if (exportObject(node, false))
                            ++num_exported;
                    });
                }
            }
        }
    }
    else if (scope == SendScope::Selected) {
        MSelectionList list;
        MGlobal::getActiveSelectionList(list);
        for (uint32_t i = 0; i < list.length(); i++) {
            MDagPath node;
            list.getDagPath(i, node);
            if (exportObject(node, false))
                ++num_exported;
        }
    }

    if (num_exported > 0 || m_deleted.length() > 0) {
        kickAsyncSend();
        return true;
    }
    else {
        return false;
    }
}

bool MeshSyncClientMaya::sendAnimations(SendScope scope)
{
    // wait for previous request to complete
    if (m_future_send.valid()) {
        m_future_send.get();
    }

    if (exportAnimations(scope) > 0) {
        kickAsyncSend();
        return true;
    }
    else {
        return false;
    }
}

void MeshSyncClientMaya::update()
{
    if (m_ignore_update)
        return;

    if (m_scene_updated) {
        m_scene_updated = false;

        registerNodeCallbacks();
        if (m_settings.auto_sync) {
            m_pending_send_scene = SendScope::All;
        }
    }

    if (m_pending_send_scene != SendScope::None) {
        sendScene(m_pending_send_scene);
    }
    else if (m_settings.auto_sync) {
        sendScene(SendScope::Updated);
    }
}

void MeshSyncClientMaya::onSelectionChanged()
{
}

void MeshSyncClientMaya::onSceneUpdated()
{
    m_scene_updated = true;
}

void MeshSyncClientMaya::onTimeChange(MTime & time)
{
    if (m_settings.auto_sync) {
        m_pending_send_scene = SendScope::All;
        // timer callback won't be called while scrubbing time slider. so call update() immediately
        update();
    }
}

void MeshSyncClientMaya::onNodeRemoved(MObject & node)
{
    if (node.hasFn(MFn::kTransform)) {
        auto it = m_object_records.find((void*&)node);
        if (it != m_object_records.end()) {
            Each(it->second.dagpaths_prev, [this](const MDagPath& dp) {
                m_deleted.append(dp);
            });
            m_object_records.erase(it);
        }
    }
}

void MeshSyncClientMaya::kickAsyncSend()
{
    if (!m_extract_records.empty()) {
        mu::parallel_for_each(m_extract_records.begin(), m_extract_records.end(), [](ExtractRecords::value_type& kvp) {
            kvp.second.processTasks();
        });
        m_extract_records.clear();
    }

    m_material_id_table.clear();

    for (auto& kvp : m_object_records) {
        kvp.second.clear();
    }

    float to_meter = 1.0f;
    {
        MDistance dist;
        dist.setValue(1.0f);
        to_meter = (float)dist.asMeters();
    }

    m_future_send = std::async(std::launch::async, [this, to_meter]() {
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
        uint32_t num_deleted = m_deleted.length();
        if(num_deleted > 0) {
            ms::DeleteMessage del;
            del.targets.resize(num_deleted);
            for (uint32_t i = 0; i < num_deleted; ++i)
                del.targets[i].path = GetPath(m_deleted[i]);
            m_deleted.clear();

            client.send(del);
        }

        // send scene data
        {
            ms::SetMessage set;
            set.scene.settings  = scene_settings;
            set.scene.objects = m_client_objects;
            set.scene.materials = m_client_materials;
            client.send(set);

            m_client_objects.clear();
            m_client_materials.clear();
        }

        // send meshes one by one to Unity can respond quickly
        for(auto& mesh : m_client_meshes) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.objects = { mesh };
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
