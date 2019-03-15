#include "pch.h"
#include "msmayaUtils.h"
#include "MeshSyncClientMaya.h"
#include "msmayaCommands.h"

#ifdef _WIN32
#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "OpenMaya.lib")
#pragma comment(lib, "OpenMayaAnim.lib")
#endif


bool DAGNode::isInstance() const
{
    return branches.size() > 1;
}

ms::Identifier TreeNode::getIdentifier() const
{
    return { path, id };
}

void TreeNode::clearState()
{
    dst_obj = nullptr;
    dst_anim = nullptr;
}

bool TreeNode::isInstance() const
{
    return shape->branches.size() > 1 ||
        trans->branches.size() > 1;
}

TreeNode* TreeNode::getPrimaryInstanceNode() const
{
    auto get_primary_parent = [](const MObject& node) -> MObject {
        Pad<MFnDagNode> dn(node);
        return dn.parentCount() > 0 ? dn.parent(0) : MObject();
    };

    if (trans->branches.size() > 1) {
        auto primary = get_primary_parent(trans->node);
        for (auto b : trans->branches) {
            if (b->parent->trans->node == primary)
                return b;
        }
    }
    if (shape->branches.size() > 1) {
        auto primary = get_primary_parent(shape->node);
        for (auto b : shape->branches) {
            if (b->trans->node == primary)
                return b;
        }
    }
    return nullptr;
}

MDagPath TreeNode::getDagPath() const
{
    MDagPath ret;
    getDagPath_(ret);
    if (shape)
        ret.push(shape->node);
    return ret;
}
void TreeNode::getDagPath_(MDagPath & dst) const
{
    if (parent) {
        parent->getDagPath_(dst);
        dst.push(trans->node);
    }
    else {
        MDagPath::getAPathTo(trans->node, dst);
    }
}

MObject TreeNode::getTrans() const { return trans ? trans->node : MObject(); }
MObject TreeNode::getShape() const { return shape ? shape->node : MObject(); }

MDagPath GetDagPath(const TreeNode *branch, const MObject& node)
{
    MDagPath ret;
    branch->getDagPath_(ret);
    ret.push(node);
    return ret;
}

TreeNode* FindBranch(const DAGNodeMap& dnmap, const MDagPath& dagpath)
{
    auto node = dagpath.node();
    auto it = dnmap.find(node);
    if (it != dnmap.end()) {
        const auto& dn = it->second;
        if (dn.branches.size() == 1) {
            return dn.branches.front();
        }
        else {
            for (auto *branch : dn.branches) {
                if (branch->getDagPath() == dagpath) {
                    return branch;
                }
            }
        }
    }

    return nullptr;
}


static void OnIdle(float elapsedTime, float lastTime, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->update();
}

static void OnSceneLoadBegin(void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onSceneLoadBegin();
}

static void OnSceneLoadEnd(void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onSceneLoadEnd();
}

static void OnNodeRenamed(void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onNodeRenamed();
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
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onNodeUpdated(plug.node());
}

static void OnCameraUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onNodeUpdated(plug.node());
}

static void OnLightUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onNodeUpdated(plug.node());
}

static void OnMeshUpdated(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other_plug, void *_this)
{
    reinterpret_cast<MeshSyncClientMaya*>(_this)->onNodeUpdated(plug.node());
}


void MeshSyncClientMaya::onNodeUpdated(const MObject& node)
{
    m_dag_nodes[node].dirty = true;
}

void MeshSyncClientMaya::onNodeRemoved(const MObject& node)
{
    {
        auto it = m_dag_nodes.find(node);
        if (it != m_dag_nodes.end()) {
            for (auto tn : it->second.branches)
                m_entity_manager.erase(tn->getIdentifier());
            m_dag_nodes.erase(it);
        }
    }
}

void MeshSyncClientMaya::onNodeRenamed()
{
    bool needs_update = false;
    for (auto& tn : m_tree_nodes) {
        if (checkRename(tn.get()))
            needs_update = true;
    }

    if (needs_update) {
        m_scene_updated = true;
    }
}

void MeshSyncClientMaya::onSceneUpdated()
{
    m_scene_updated = true;
}

void MeshSyncClientMaya::onSceneLoadBegin()
{
    m_ignore_update = true;
}

void MeshSyncClientMaya::onSceneLoadEnd()
{
    m_ignore_update = false;
}

void MeshSyncClientMaya::onTimeChange(const MTime & time)
{
    if (m_settings.auto_sync) {
        m_pending_scope = SendScope::All;
        // timer callback won't be called while scrubbing time slider. so call update() immediately
        update();

        // for timer callback
        m_pending_scope = SendScope::All;
    }
}


void MeshSyncClientMaya::TaskRecord::add(TreeNode *n, const task_t & task)
{
    tasks.push_back({ n, task });
}

void MeshSyncClientMaya::TaskRecord::process()
{
    if (tasks.size() > 1) {
        auto primary = std::get<0>(tasks[0])->getPrimaryInstanceNode();

        // process primary node first
        for (auto& t : tasks) {
            if (std::get<0>(t) == primary) {
                std::get<1>(t)();
                break;
            }
        }
        // process others
        for (auto& t : tasks) {
            if (std::get<0>(t) != primary)
                std::get<1>(t)();
        }
    }
    else {
        for (auto& t : tasks)
            std::get<1>(t)();
    }

    tasks.clear();
}


static std::unique_ptr<MeshSyncClientMaya> g_plugin;


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
}

MeshSyncClientMaya::~MeshSyncClientMaya()
{
    m_sender.wait();
    removeNodeCallbacks();
    removeGlobalCallbacks();
#define Body(CmdType) m_iplugin.deregisterCommand(CmdType::name());
    EachCommand(Body)
#undef Body
}

void MeshSyncClientMaya::registerGlobalCallbacks()
{
    MStatus stat;
    m_cids_global.push_back(MTimerMessage::addTimerCallback(0.03f, OnIdle, this, &stat));
    m_cids_global.push_back(MSceneMessage::addCallback(MSceneMessage::kBeforeNew, OnSceneLoadBegin, this, &stat));
    m_cids_global.push_back(MSceneMessage::addCallback(MSceneMessage::kBeforeOpen, OnSceneLoadBegin, this, &stat));
    m_cids_global.push_back(MSceneMessage::addCallback(MSceneMessage::kAfterNew, OnSceneLoadEnd, this, &stat));
    m_cids_global.push_back(MSceneMessage::addCallback(MSceneMessage::kAfterOpen, OnSceneLoadEnd, this, &stat));
    m_cids_global.push_back(MDagMessage::addAllDagChangesCallback(OnDagChange, this, &stat));
    m_cids_global.push_back(MEventMessage::addEventCallback("NameChanged", OnNodeRenamed, this, &stat));
    m_cids_global.push_back(MDGMessage::addForceUpdateCallback(OnTimeChange, this));
    m_cids_global.push_back(MDGMessage::addNodeRemovedCallback(OnNodeRemoved, kDefaultNodeType, this));

    // shut up warning about blendshape
    MGlobal::executeCommand("cycleCheck -e off");
}

void MeshSyncClientMaya::registerNodeCallbacks()
{
    // joints
    EnumerateNode(MFn::kJoint, [this](MObject& node) {
        auto& rec = m_dag_nodes[node];
        if (!rec.cid)
            rec.cid = MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this);
    });

    auto register_parent_transforms = [this](MObject& n) {
        EachParentRecursive(n, [this](MObject parent) {
            if (parent.hasFn(MFn::kTransform)) {
                auto& rec = m_dag_nodes[parent];
                if (!rec.cid)
                    rec.cid = MNodeMessage::addAttributeChangedCallback(parent, OnTransformUpdated, this);
            }
        });
    };

    // cameras
    EnumerateNode(MFn::kCamera, [&](MObject& node) {
        Pad<MFnDagNode> fn(node);
        if (!fn.isIntermediateObject()) {
            register_parent_transforms(node);
            auto& rec = m_dag_nodes[node];
            if (!rec.cid)
                rec.cid = MNodeMessage::addAttributeChangedCallback(node, OnCameraUpdated, this);
        }
    });

    // lights
    EnumerateNode(MFn::kLight, [&](MObject& node) {
        Pad<MFnDagNode> fn(node);
        if (!fn.isIntermediateObject()) {
            register_parent_transforms(node);
            auto& rec = m_dag_nodes[node];
            if (!rec.cid)
                rec.cid = MNodeMessage::addAttributeChangedCallback(node, OnLightUpdated, this);
        }
    });

    //  meshes
    EnumerateNode(MFn::kMesh, [&](MObject& node) {
        Pad<MFnDagNode> fn(node);
        if (!fn.isIntermediateObject()) {
            register_parent_transforms(node);
            auto& rec = m_dag_nodes[node];
            if (!rec.cid)
                m_dag_nodes[node].cid = MNodeMessage::addAttributeChangedCallback(node, OnMeshUpdated, this);
        }
    });
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
    for (auto& rec : m_dag_nodes) {
        if (rec.second.cid) {
            MMessage::removeCallback(rec.second.cid);
            rec.second.cid = 0;
        }
    }
}


void MeshSyncClientMaya::constructTree()
{
    m_tree_roots.clear();
    m_tree_nodes.clear();
    for (auto& kvp : m_dag_nodes)
        kvp.second.branches.clear();
    m_index_seed = 0;

    EnumerateNode(MFn::kTransform, [&](MObject& node) {
        if (Pad<MFnDagNode>(node).parent(0).hasFn(MFn::kWorld)) {
            constructTree(node, nullptr, "");
        }
    });
}

void MeshSyncClientMaya::constructTree(const MObject& node, TreeNode *parent, const std::string& base)
{
    MObject shape;
    {
        MDagPath dpath;
        MDagPath::getAPathTo(node, dpath);
        dpath.extendToShape();
        shape = dpath.node();
    }

    std::string name = GetName(node);
    std::string path = base;
    path += '/';
    path += name;

    auto& rec_node = m_dag_nodes[node];
    auto& rec_shape = m_dag_nodes[shape];
    if (rec_node.node.isNull()) {
        rec_node.node = node;
        rec_shape.node = shape;
    }

    auto *tn = new TreeNode();
    m_tree_nodes.emplace_back(tn);
    tn->trans = &rec_node;
    tn->shape = &rec_shape;
    tn->name = name;
    tn->path = path;
    tn->index = ++m_index_seed;
    tn->parent = parent;

    if (parent)
        parent->children.push_back(tn);
    else
        m_tree_roots.push_back(tn);

    rec_node.branches.push_back(tn);
    if (shape != node)
        rec_shape.branches.push_back(tn);

    EachChild(node, [&](const MObject & c) {
        if (c.hasFn(MFn::kTransform))
            constructTree(c, tn, path);
    });
}

bool MeshSyncClientMaya::checkRename(TreeNode *tn)
{
    if (tn->name != GetName(tn->trans->node)) {
        m_entity_manager.erase(tn->getIdentifier());
        return true;
    }
    else {
        for (auto child : tn->children) {
            if (checkRename(child))
                return true;
        }
    }
    return false;
}


bool MeshSyncClientMaya::sendScene(SendScope scope, bool dirty_all)
{
    if (m_sender.isSending()) {
        m_pending_scope = scope;
        return false;
    }
    m_pending_scope = SendScope::None;

    if (dirty_all) {
        m_material_manager.makeDirtyAll();
        m_entity_manager.makeDirtyAll();
    }

    if (m_settings.sync_meshes)
        exportMaterials();

    int num_exported = 0;
    auto export_branches = [&](DAGNode& rec) {
        for (auto *tn : rec.branches) {
            if (exportObject(tn, true))
                ++num_exported;
        }
        rec.dirty = false;
    };

    if (scope == SendScope::All) {
        //EnumerateAllNode([](MObject& obj) { PrintNodeInfo(obj); });

        auto handler = [&](MObject& node) {
            export_branches(m_dag_nodes[node]);
        };
        EnumerateNode(MFn::kJoint, handler);
        EnumerateNode(MFn::kCamera, handler);
        EnumerateNode(MFn::kLight, handler);
        EnumerateNode(MFn::kMesh, handler);
    }
    else if (scope == SendScope::Updated) {
        for (auto& kvp : m_dag_nodes) {
            auto& rec = kvp.second;
            if (rec.dirty)
                export_branches(rec);
        }
    }
    else if (scope == SendScope::Selected) {
        MSelectionList list;
        MGlobal::getActiveSelectionList(list);
        for (uint32_t i = 0; i < list.length(); i++) {
            MObject node;
            list.getDependNode(i, node);
            export_branches(m_dag_nodes[node]);
        }
    }

    if (num_exported > 0 || !m_entity_manager.getDeleted().empty()) {
        kickAsyncSend();
        return true;
    }
    else {
        return false;
    }
}

bool MeshSyncClientMaya::sendAnimations(SendScope scope)
{
    m_sender.wait();

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

        constructTree();
        registerNodeCallbacks();
        if (m_settings.auto_sync) {
            m_pending_scope = SendScope::All;
        }
    }

    if (m_pending_scope != SendScope::None) {
        sendScene(m_pending_scope, false);
    }
    else if (m_settings.auto_sync) {
        sendScene(SendScope::Updated, false);
    }
}


void MeshSyncClientMaya::kickAsyncSend()
{
    // process parallel extract tasks
    if (!m_extract_tasks.empty()) {
        if (m_settings.multithreaded) {
            mu::parallel_for_each(m_extract_tasks.begin(), m_extract_tasks.end(), [](TaskRecords::value_type& kvp) {
                kvp.second.process();
            });
        }
        else {
            for (auto& kvp : m_extract_tasks) {
                kvp.second.process();
            }
        }
        m_extract_tasks.clear();
    }

    // cleanup
    for (auto& n : m_tree_nodes)
        n->clearState();

    float to_meter = 1.0f;
    {
        MDistance dist;
        dist.setValue(1.0f);
        to_meter = (float)dist.asMeters();
    }

    m_sender.on_prepare = [this, to_meter]() {
        auto& t = m_sender;
        t.client_settings = m_settings.client_settings;
        t.scene_settings.handedness = ms::Handedness::Right;
        t.scene_settings.scale_factor = m_settings.scale_factor / to_meter;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();
        t.animations = m_animations;

        t.deleted_materials = m_material_manager.getDeleted();
        t.deleted_entities = m_entity_manager.getDeleted();
    };
    m_sender.on_success = [this]() {
        m_material_ids.clearDirtyFlags();
        m_material_manager.clearDirtyFlags();
        m_texture_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
        m_animations.clear();
    };
    m_sender.kick();
}

bool MeshSyncClientMaya::recvScene()
{
    m_sender.wait();

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


    // todo: 

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
