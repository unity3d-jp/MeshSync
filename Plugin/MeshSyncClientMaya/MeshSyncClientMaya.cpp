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
        m_future_send.wait_for(std::chrono::milliseconds(m_timeout_ms));
    }
}

void MeshSyncClientMaya::registerGlobalCallbacks()
{
    MStatus stat;
    m_cids_global.push_back(MTimerMessage::addTimerCallback(0.03f, OnIdle, this, &stat));
    m_cids_global.push_back(MEventMessage::addEventCallback("SelectionChanged", OnSelectionChanged, this, &stat));
    m_cids_global.push_back(MDagMessage::addAllDagChangesCallback(OnDagChange, this, &stat));
    m_cids_global.push_back(MDGMessage::addTimeChangeCallback(OnTimeChange, this));
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
                m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this, &stat));
            }
            else {
                auto path_to_shape = path;
                if (path_to_shape.extendToShape() == MS::kSuccess && path_to_shape.node().hasFn(MFn::kMesh)) {
                    mscTrace("tracking mesh %s\n", path.fullPathName().asChar());
                    m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(node, OnTransformUpdated, this, &stat));

                    MObject shape_obj = path_to_shape.node();
                    m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(shape_obj, OnMeshUpdated, this, &stat));
                }
            }
        }
    }
    else {
        // track  meshes
        {
            MItDag it(MItDag::kDepthFirst, MFn::kMesh);
            while (!it.isDone()) {
                auto shape = it.item();
                MFnMesh fn(shape);
                if (!fn.isIntermediateObject()) {
                    auto trans = GetTransform(shape);
                    mscTrace("tracking mesh %s\n", GetDagPath(trans).fullPathName().asChar());
                    m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(trans, OnTransformUpdated, this, &stat));
                    m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(shape, OnMeshUpdated, this, &stat));
                }
                it.next();
            }
        }

        // track cameras
        {
            MItDag it(MItDag::kDepthFirst, MFn::kCamera);
            while (!it.isDone()) {
                auto shape = it.item();
                MFnCamera fn(shape);
                if (!fn.isIntermediateObject()) {
                    auto trans = GetTransform(shape);
                    mscTrace("tracking camera %s\n", GetDagPath(trans).fullPathName().asChar());
                    m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(trans, OnTransformUpdated, this, &stat));
                    m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(shape, OnCameraUpdated, this, &stat));
                }
                it.next();
            }
        }

        // track lights
        {
            MItDag it(MItDag::kDepthFirst, MFn::kLight);
            while (!it.isDone()) {
                auto shape = it.item();
                MFnLight fn(shape);
                if (!fn.isIntermediateObject()) {
                    auto trans = GetTransform(shape);
                    mscTrace("tracking light %s\n", GetDagPath(trans).fullPathName().asChar());
                    m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(trans, OnTransformUpdated, this, &stat));
                    m_cids_node.push_back(MNodeMessage::addAttributeChangedCallback(shape, OnLightUpdated, this, &stat));
                }
                it.next();
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

void MeshSyncClientMaya::removeNodeCallbacks()
{
    for (auto& cid : m_cids_node) {
        MMessage::removeCallback(cid);
    }
    m_cids_node.clear();
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

int MeshSyncClientMaya::getObjectID(MUuid uid)
{
    std::unique_lock<std::mutex> lock(m_mutex_extract_mesh);

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

void MeshSyncClientMaya::notifyUpdateTransform(MObject node, bool force)
{
    if ((force || m_auto_sync) && node.hasFn(MFn::kTransform)) {
        if (std::find(m_mtransforms.begin(), m_mtransforms.end(), node) == m_mtransforms.end()) {
            mscTrace("MeshSyncClientMaya::notifyUpdateTransform()\n");
            m_mtransforms.push_back(node);
        }
    }
}

void MeshSyncClientMaya::notifyUpdateCamera(MObject shape, bool force)
{
    if ((force || m_auto_sync) && m_sync_cameras && shape.hasFn(MFn::kCamera)) {
        auto node = GetTransform(shape);
        if (std::find(m_mcameras.begin(), m_mcameras.end(), node) == m_mcameras.end()) {
            mscTrace("MeshSyncClientMaya::notifyUpdateCamera()\n");
            m_mcameras.push_back(node);
        }
    }
}

void MeshSyncClientMaya::notifyUpdateLight(MObject shape, bool force)
{
    if ((force || m_auto_sync) && m_sync_lights && shape.hasFn(MFn::kLight)) {
        auto node = GetTransform(shape);
        if (std::find(m_mlights.begin(), m_mlights.end(), node) == m_mlights.end()) {
            mscTrace("MeshSyncClientMaya::notifyUpdateLight()\n");
            m_mlights.push_back(node);
        }
    }
}

void MeshSyncClientMaya::notifyUpdateMesh(MObject shape, bool force)
{
    if ((force || m_auto_sync) && m_sync_meshes && shape.hasFn(MFn::kMesh)) {
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
    mscTrace("MeshSyncClientMaya::onSceneUpdated()\n");
}

void MeshSyncClientMaya::onTimeChange(MTime & time)
{
    sendScene();
    mscTrace("MeshSyncClientMaya::onTimeChange()\n");
}

bool MeshSyncClientMaya::sendUpdatedObjects()
{
    if (isAsyncSendInProgress() ||
        (m_mtransforms.empty() && m_mcameras.empty() && m_mlights.empty() && m_mmeshes.empty())) {
        return false;
    }

    if (!m_mmeshes.empty()) {
        extractAllMaterialData();
    }
    for (auto& mmesh : m_mmeshes) {
        auto mesh = ms::MeshPtr(new ms::Mesh());
        if (extractMeshData(*mesh, mmesh)) {
            m_client_meshes.emplace_back(mesh);
        }
    }

    for (auto& mcam : m_mcameras) {
        auto dst = ms::CameraPtr(new ms::Camera());
        if (extractCameraData(*dst, mcam)) {
            m_client_cameras.emplace_back(dst);
        }
    }

    for (auto& mlight : m_mlights) {
        auto dst = ms::LightPtr(new ms::Light());
        if (extractLightData(*dst, mlight)) {
            m_client_lights.emplace_back(dst);
        }
    }

    // transforms must be latter as extractMeshData() maybe add joints to m_mtransforms
    for (auto& mtrans : m_mtransforms) {
        auto dst = ms::TransformPtr(new ms::Transform());
        if (extractTransformData(*dst, mtrans)) {
            m_client_transforms.emplace_back(dst);
        }
    }

    m_mtransforms.clear();
    m_mcameras.clear();
    m_mlights.clear();
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

    // clear dirty list and add all objects
    m_mtransforms.clear();
    m_mmeshes.clear();

    if(scope == TargetScope::All) {
        // meshes
        {
            MItDag it(MItDag::kDepthFirst, MFn::kMesh);
            while (!it.isDone()) {
                auto shape = it.item();
                MFnMesh fn(shape);
                if (!fn.isIntermediateObject()) {
                    notifyUpdateMesh(shape, true);
                }

                it.next();
            }
        }

        // cameras
        {
            MItDag it(MItDag::kDepthFirst, MFn::kCamera);
            while (!it.isDone()) {
                auto shape = it.item();
                MFnCamera fn(shape);
                if (!fn.isIntermediateObject()) {
                    notifyUpdateCamera(shape, true);
                }
                it.next();
            }
        }

        // lights
        {
            MItDag it(MItDag::kDepthFirst, MFn::kLight);
            while (!it.isDone()) {
                auto shape = it.item();
                MFnLight fn(shape);
                if (!fn.isIntermediateObject()) {
                    notifyUpdateLight(shape, true);
                }
                it.next();
            }
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

    return sendUpdatedObjects();
}

bool MeshSyncClientMaya::importScene()
{
    waitAsyncSend();

    ms::Client client(m_client_settings);
    ms::GetMessage gd;
    gd.flags.get_transform = 1;
    gd.flags.get_indices = 1;
    gd.flags.get_points = 1;
    gd.flags.get_uv0 = 1;
    gd.flags.get_uv1 = 1;
    gd.flags.get_material_ids= 1;
    gd.scene_settings.handedness = ms::Handedness::Right;
    gd.scene_settings.scale_factor = 1.0f / m_scale_factor;
    gd.refine_settings.flags.bake_skin = m_bake_skin;
    gd.refine_settings.flags.bake_cloth = m_bake_cloth;

    auto ret = client.send(gd);
    if (!ret) {
        return false;
    }


    // todo: create mesh objects

    return true;
}

void MeshSyncClientMaya::extractAllMaterialData()
{
    MItDependencyNodes it(MFn::kLambert);
    while (!it.isDone()) {
        MFnLambertShader fn(it.item());

        auto tmp = new ms::Material();
        tmp->name = fn.name().asChar();
        tmp->color = to_float4(fn.color());
        tmp->id = getMaterialID(fn.uuid());
        m_client_materials.emplace_back(tmp);

        it.next();
    }
}

bool MeshSyncClientMaya::extractTransformData(ms::Transform& dst, MObject src)
{
    MFnTransform mtrans(src);

    MStatus stat;
    MVector pos;
    MQuaternion rot;
    double scale[3];

    dst.path = GetPath(src);
    pos = mtrans.getTranslation(MSpace::kTransform, &stat);
    stat = mtrans.getRotation(rot, MSpace::kTransform);
    stat = mtrans.getScale(scale);
    dst.position = to_float3(pos);
    dst.rotation = to_quatf(rot);
    dst.scale = to_float3(scale);

    // handle joint's segment scale compensate
    if (src.hasFn(MFn::kJoint)) {
        mu::float3 inverse_scale;
        if (JointGetSegmentScaleCompensate(src) && JointGetInverseScale(src, inverse_scale)) {
            dst.scale /= inverse_scale;
        }
    }

    // handle animation
    if (m_sync_animations && MAnimUtil::isAnimated(src)) {
        // get TRS & visibility animation plugs
        MPlugArray plugs;
        MAnimUtil::findAnimatedPlugs(src, plugs);
        auto num_plugs = plugs.length();

        MPlug ptx, pty, ptz, prx, pry, prz, psx, psy, psz, pvis;
        int found = 0;
        for (uint32_t pi = 0; pi < num_plugs; ++pi) {
            auto plug = plugs[pi];
            MObjectArray animation;
            if (!MAnimUtil::findAnimation(plug, animation)) { continue; }

            std::string name = plug.name().asChar();
#define Case(Name, Plug) if (name.find(Name) != std::string::npos) { Plug = plug; ++found; continue; }
            Case(".translateX", ptx);
            Case(".translateY", pty);
            Case(".translateZ", ptz);
            Case(".scaleX", psx);
            Case(".scaleY", psy);
            Case(".scaleZ", psz);
            Case(".rotateX", prx);
            Case(".rotateY", pry);
            Case(".rotateZ", prz);
            Case(".visibility", pvis);
#undef Case
        }

        // skip if no animation plugs are found
        if (found > 0) {
            dst.createAnimation();
            auto& anim = dynamic_cast<ms::TransformAnimation&>(*dst.animation);

            // build time-sampled animation data
            int sps = m_sample_animation ? m_animation_sps : 0;
            ConvertAnimationBool(anim.visible, true, pvis, sps);
            ConvertAnimationFloat3(anim.translation, dst.position, ptx, pty, ptz, sps);
            ConvertAnimationFloat3(anim.scale, dst.scale, psx, psy, psz, sps);
            {
                // build rotation animation data (eular angles) and convert to quaternion
                RawVector<ms::TVP<mu::float3>> eular;
                ConvertAnimationFloat3(eular, mu::float3::zero(), prx, pry, prz, sps);

                if (!eular.empty()) {
                    anim.rotation.resize(eular.size());
                    size_t n = eular.size();

#define Case(Order)\
    case MTransformationMatrix::k##Order:\
        for (size_t i = 0; i < n; ++i) {\
            anim.rotation[i].time = eular[i].time;\
            anim.rotation[i].value = mu::rotate##Order(eular[i].value);\
        }\
        break;

                    MFnTransform mtrans(src);
                    switch (mtrans.rotationOrder()) {
                        Case(XYZ);
                        Case(YZX);
                        Case(ZXY);
                        Case(XZY);
                        Case(YXZ);
                        Case(ZYX);
                        default: break;
                    }
#undef Case
                }
            }

            if (dst.animation->empty()) {
                dst.animation.reset();
            }
        }
    }
    return true;
}

bool MeshSyncClientMaya::extractCameraData(ms::Camera& dst, MObject src)
{
    if (!extractTransformData(dst, src)) { return false; }
    dst.rotation = mu::flipY(dst.rotation);

    auto shape = GetShape(src);
    if (!shape.hasFn(MFn::kCamera)) {
        return false;
    }

    MFnCamera mcam(shape);
    dst.is_ortho = mcam.isOrtho();
    dst.near_plane = (float)mcam.nearClippingPlane();
    dst.far_plane = (float)mcam.farClippingPlane();
    dst.fov = (float)mcam.horizontalFieldOfView() * ms::Rad2Deg;

    dst.horizontal_aperture = (float)mcam.horizontalFilmAperture() * InchToMillimeter;
    dst.vertical_aperture = (float)mcam.verticalFilmAperture() * InchToMillimeter;
    dst.focal_length = (float)mcam.focalLength();
    dst.focus_distance = (float)mcam.focusDistance();

    // handle animation
    if (m_sync_animations && MAnimUtil::isAnimated(shape)) {
        MPlugArray plugs;
        MAnimUtil::findAnimatedPlugs(shape, plugs);
        auto num_plugs = plugs.length();

        MPlug pnplane, pfplane, phaperture, pvaperture, pflen, pfdist;
        int found = 0;
        for (uint32_t pi = 0; pi < num_plugs; ++pi) {
            auto plug = plugs[pi];
            MObjectArray animation;
            if (!MAnimUtil::findAnimation(plug, animation)) { continue; }

            std::string name = plug.name().asChar();
#define Case(Name, Plug) if (name.find(Name) != std::string::npos) { Plug = plug; ++found; continue; }
            Case(".nearClipPlane", pnplane);
            Case(".farClipPlane", pfplane);
            Case(".horizontalFilmAperture", phaperture);
            Case(".verticalFilmAperture", pvaperture);
            Case(".focalLength", pflen);
            Case(".focusDistance", pfdist);
#undef Case
        }

        // skip if no animation plugs are found
        if (found > 0) {
            dst.createAnimation();
            auto& anim = dynamic_cast<ms::CameraAnimation&>(*dst.animation);

            // build time-sampled animation data
            int sps = m_sample_animation ? m_animation_sps : 0;
            ConvertAnimationFloat(anim.near_plane, dst.near_plane, pnplane, sps);
            ConvertAnimationFloat(anim.far_plane, dst.far_plane, pfplane, sps);
            ConvertAnimationFloat(anim.horizontal_aperture, dst.horizontal_aperture, phaperture, sps);
            ConvertAnimationFloat(anim.vertical_aperture, dst.vertical_aperture, pvaperture, sps);
            ConvertAnimationFloat(anim.focal_length, dst.focal_length, pflen, sps);
            ConvertAnimationFloat(anim.focus_distance, dst.focus_distance, pfdist, sps);

            // convert inch to millimeter
            for (auto& v : anim.horizontal_aperture) { v.value *= InchToMillimeter; }
            for (auto& v : anim.vertical_aperture) { v.value *= InchToMillimeter; }

            // fov needs calculate by myself...
            if (!anim.focal_length.empty() || !anim.horizontal_aperture.empty()) {
                MFnAnimCurve fcv, acv;
                GetAnimationCurve(fcv, pflen);
                GetAnimationCurve(acv, phaperture);

                auto time_samples = BuildTimeSamples({ ptr(fcv), ptr(acv) }, sps);
                auto num_samples = time_samples.size();
                anim.fov.resize(num_samples);
                if(!fcv.object().isNull() && acv.object().isNull()) {
                    for (size_t i = 0; i < num_samples; ++i) {
                        auto& tvp = anim.fov[i];
                        tvp.time = time_samples[i];
                        tvp.value = mu::compute_fov(
                            dst.horizontal_aperture,
                            (float)fcv.evaluate(ToMTime(time_samples[i])));
                    }
                }
                else if (fcv.object().isNull() && !acv.object().isNull()) {
                    for (size_t i = 0; i < num_samples; ++i) {
                        auto& tvp = anim.fov[i];
                        tvp.time = time_samples[i];
                        tvp.value = mu::compute_fov(
                            (float)acv.evaluate(ToMTime(time_samples[i])),
                            dst.focal_length);
                    }
                }
                else if (!fcv.object().isNull() && !acv.object().isNull()) {
                    for (size_t i = 0; i < num_samples; ++i) {
                        auto& tvp = anim.fov[i];
                        tvp.time = time_samples[i];
                        tvp.value = mu::compute_fov(
                            (float)acv.evaluate(ToMTime(time_samples[i])),
                            (float)fcv.evaluate(ToMTime(time_samples[i])));
                    }
                }
            }

            if (dst.animation->empty()) {
                dst.animation.reset();
            }
        }
    }
    if (dst.animation) {
        auto& anim = dynamic_cast<ms::TransformAnimation&>(*dst.animation);
        for (auto& v : anim.rotation) { v.value = mu::flipY(v.value); }
    }
    return true;
}

bool MeshSyncClientMaya::extractLightData(ms::Light& dst, MObject src)
{
    if (!extractTransformData(dst, src)) { return false; }
    dst.rotation = mu::flipY(dst.rotation);

    auto shape = GetShape(src);
    if (shape.hasFn(MFn::kSpotLight)) {
        MFnSpotLight mlight(shape);
        dst.type = ms::Light::Type::Spot;
        dst.spot_angle = (float)mlight.coneAngle() * mu::Rad2Deg;
    }
    else if (shape.hasFn(MFn::kDirectionalLight)) {
        MFnDirectionalLight mlight(shape);
        dst.type = ms::Light::Type::Directional;
    }
    else if (shape.hasFn(MFn::kPointLight)) {
        MFnPointLight mlight(shape);
        dst.type = ms::Light::Type::Point;
    }
    else {
        return false;
    }

    MFnLight mlight(shape);
    auto color = mlight.color();
    dst.color = { color.r, color.g, color.b, color.a };
    dst.intensity = mlight.intensity();

    // handle animation
    if (m_sync_animations && MAnimUtil::isAnimated(shape)) {
        MPlugArray plugs;
        MAnimUtil::findAnimatedPlugs(shape, plugs);
        auto num_plugs = plugs.length();

        MPlug pcolr, pcolg, pcolb, pcola, pint;
        int found = 0;
        for (uint32_t pi = 0; pi < num_plugs; ++pi) {
            auto plug = plugs[pi];
            MObjectArray animation;
            if (!MAnimUtil::findAnimation(plug, animation)) { continue; }

            std::string name = plug.name().asChar();
#define Case(Name, Plug) if (name.find(Name) != std::string::npos) { Plug = plug; ++found; continue; }
            Case(".colorR", pcolr);
            Case(".colorG", pcolg);
            Case(".colorB", pcolb);
            Case(".intensity", pint);
#undef Case
        }

        // skip if no animation plugs are found
        if (found > 0) {
            dst.createAnimation();
            auto& anim = dynamic_cast<ms::LightAnimation&>(*dst.animation);

            // build time-sampled animation data
            int sps = m_sample_animation ? m_animation_sps : 0;
            ConvertAnimationFloat4(anim.color, dst.color, pcolr, pcolg, pcolb, pcola, sps);
            ConvertAnimationFloat(anim.intensity, dst.intensity, pint, sps);

            if (dst.animation->empty()) {
                dst.animation.reset();
            }
        }
    }
    if (dst.animation) {
        auto& anim = dynamic_cast<ms::TransformAnimation&>(*dst.animation);
        for (auto& v : anim.rotation) { v.value = mu::flipY(v.value); }
    }
    return true;
}


bool MeshSyncClientMaya::extractMeshData(ms::Mesh& dst, MObject src)
{
    dst.clear();

    if (!extractTransformData(dst, src)) { return false; }

    dst.visible = IsVisible(src);
    if (!dst.visible) { return true; }

    auto shape = GetShape(src);
    if (!shape.hasFn(MFn::kMesh)) { return false; }

    MFnMesh mmesh(shape);

    dst.flags.has_refine_settings = 1;
    dst.flags.apply_trs = 1;
    dst.refine_settings.flags.gen_tangents = 1;
    dst.refine_settings.flags.swap_faces = 1;

    if (!mmesh.object().hasFn(MFn::kMesh)) {
        // this mesh is empty
        return true;
    }

    MFnMesh fn_src_mesh(mmesh.object());
    MFnBlendShapeDeformer fn_blendshape(FindBlendShape(mmesh.object()));
    MFnSkinCluster fn_skin(FindSkinCluster(mmesh.object()));
    int skin_index = 0;

    // if target is skinned or has blend shape, use pre-deformed mesh as source
    if (m_sync_blendshapes && !fn_blendshape.object().isNull()) {
        auto orig_mesh = FindOrigMesh(src);
        if (orig_mesh.hasFn(MFn::kMesh)) {
            fn_src_mesh.setObject(orig_mesh);
        }
    }
    if (m_sync_bones && !fn_skin.object().isNull()) {
        auto orig_mesh = FindOrigMesh(src);
        if (orig_mesh.hasFn(MFn::kMesh)) {
            fn_src_mesh.setObject(orig_mesh);
            skin_index = fn_skin.indexForOutputShape(mmesh.object());
        }
    }

    // get points
    {
        MFloatPointArray points;
        if (fn_src_mesh.getPoints(points) != MStatus::kSuccess) {
            // this mesh is empty
            return true;
        }

        auto len = points.length();
        dst.points.resize(len);
        const MFloatPoint *points_ptr = &points[0];
        for (uint32_t i = 0; i < len; ++i) {
            dst.points[i] = to_float3(points_ptr[i]);
        }
    }

    // get faces
    {
        MItMeshPolygon it_poly(fn_src_mesh.object());
        dst.counts.reserve(it_poly.count());
        dst.indices.reserve(it_poly.count() * 4);

        while (!it_poly.isDone()) {
            int count = it_poly.polygonVertexCount();
            dst.counts.push_back(count);
            for (int i = 0; i < count; ++i) {
                dst.indices.push_back(it_poly.vertexIndex(i));
            }
            it_poly.next();
        }
    }

    uint32_t vertex_count = (uint32_t)dst.points.size();
    uint32_t index_count = (uint32_t)dst.indices.size();
    uint32_t face_count = (uint32_t)dst.counts.size();

    // get normals
    if (m_sync_normals) {
        MFloatVectorArray normals;
        if (fn_src_mesh.getNormals(normals) == MStatus::kSuccess) {
            dst.normals.resize_zeroclear(index_count);
            const MFloatVector *normals_ptr = &normals[0];

            size_t ii = 0;
            MItMeshPolygon it_poly(fn_src_mesh.object());
            while (!it_poly.isDone()) {
                int count = it_poly.polygonVertexCount();
                for (int i = 0; i < count; ++i) {
                    dst.normals[ii] = to_float3(normals_ptr[it_poly.normalIndex(i)]);
                    ++ii;
                }
                it_poly.next();
            }
        }
    }

    // get uv
    if (m_sync_uvs) {
        MStringArray uvsets;
        fn_src_mesh.getUVSetNames(uvsets);

        if (uvsets.length() > 0 && fn_src_mesh.numUVs(uvsets[0]) > 0) {
            dst.uv0.resize_zeroclear(index_count);

            MFloatArray u;
            MFloatArray v;
            fn_src_mesh.getUVs(u, v, &uvsets[0]);
            const float *u_ptr = &u[0];
            const float *v_ptr = &v[0];

            size_t ii = 0;
            MItMeshPolygon it_poly(fn_src_mesh.object());
            while (!it_poly.isDone()) {
                int count = it_poly.polygonVertexCount();
                for (int i = 0; i < count; ++i) {
                    int iu;
                    if (it_poly.getUVIndex(i, iu, &uvsets[0]) == MStatus::kSuccess && iu >= 0)
                        dst.uv0[ii] = mu::float2{ u_ptr[iu], v_ptr[iu] };
                    ++ii;
                }
                it_poly.next();
            }
        }
    }

    // get vertex colors
    if (m_sync_colors) {
        MStringArray color_sets;
        fn_src_mesh.getColorSetNames(color_sets);

        if (color_sets.length() > 0 && fn_src_mesh.numColors(color_sets[0]) > 0) {
            dst.colors.resize(index_count, mu::float4::one());

            MColorArray colors;
            fn_src_mesh.getColors(colors, &color_sets[0]);
            const MColor *colors_ptr = &colors[0];

            size_t ii = 0;
            MItMeshPolygon it_poly(fn_src_mesh.object());
            while (!it_poly.isDone()) {
                int count = it_poly.polygonVertexCount();
                for (int i = 0; i < count; ++i) {
                    int ic;
                    if (it_poly.getColorIndex(i, ic, &color_sets[0]) == MStatus::kSuccess && ic >= 0)
                        dst.colors[ii] = (const mu::float4&)colors_ptr[ic];
                    ++ii;
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
        mmesh.getConnectedShaders(0, shaders, indices);
        mids.resize(shaders.length(), -1);
        for (uint32_t si = 0; si < shaders.length(); si++) {
            MItDependencyGraph it(shaders[si], MFn::kLambert, MItDependencyGraph::kUpstream);
            if (!it.isDone()) {
                MFnLambertShader lambert(it.currentItem());
                mids[si] = getMaterialID(lambert.uuid());
            }
        }

        if (mids.size() > 0) {
            dst.material_ids.resize_zeroclear(face_count);
            uint32_t len = std::min(face_count, indices.length());
            for (uint32_t i = 0; i < len; ++i) {
                dst.material_ids[i] = mids[indices[i]];
            }
        }
    }

    // get blendshape data
    if (m_sync_blendshapes && !fn_blendshape.object().isNull()) {
        // https://knowledge.autodesk.com/search-result/caas/CloudHelp/cloudhelp/2018/ENU/Maya-Tech-Docs/Nodes/blendShape-html.html

        MPlug plug_weight = fn_blendshape.findPlug("weight");
        MPlug plug_it = fn_blendshape.findPlug("inputTarget");
        uint32_t num_it = plug_it.evaluateNumElements();
        for (uint32_t idx_it = 0; idx_it < num_it; ++idx_it) {
            MPlug plug_itp(plug_it.elementByPhysicalIndex(idx_it));
            if (plug_itp.logicalIndex() == 0) {
                MPlug plug_itg(plug_itp.child(0)); // .inputTarget[idx_it].inputTargetGroup
                uint32_t num_itg = plug_itg.evaluateNumElements();
                //DumpPlugInfo(plug_itg);

                for (uint32_t idx_itg = 0; idx_itg < num_itg; ++idx_itg) {
                    auto dst_bs = new ms::BlendShapeData();
                    dst.blendshapes.emplace_back(dst_bs);
                    MPlug plug_wc = plug_weight.elementByPhysicalIndex(idx_itg);
                    dst_bs->name = plug_wc.name().asChar();
                    plug_wc.getValue(dst_bs->weight);
                    dst_bs->weight *= 100.0f;

                    MPlug plug_itgp(plug_itg.elementByPhysicalIndex(idx_itg));
                    uint32_t delta_index = plug_itgp.logicalIndex();

                    MPlug plug_iti(plug_itgp.child(0)); // .inputTarget[idx_it].inputTargetGroup[idx_itg].inputTargetItem
                    uint32_t num_iti(plug_iti.evaluateNumElements());
                    for (uint32_t idx_iti = 0; idx_iti != num_iti; ++idx_iti) {
                        MPlug plug_itip(plug_iti.elementByPhysicalIndex(idx_iti));

                        dst_bs->frames.push_back(ms::BlendShapeData::Frame());
                        auto& frame = dst_bs->frames.back();
                        frame.weight = float(plug_itip.logicalIndex() - 5000) / 10.0f;
                        frame.points.resize_zeroclear(dst.points.size());

                        bool handled = false;
                        if (!handled) {
                            MPlug plug_geom(plug_itip.child(0)); // .inputGeomTarget
                            MObject obj_geom;
                            plug_geom.getValue(obj_geom);
                            if (!obj_geom.isNull() && obj_geom.hasFn(MFn::kMesh)) {
                                handled = true;

                                MFnMesh fn_geom(obj_geom);
                                MFloatPointArray points;
                                fn_geom.getPoints(points);

                                uint32_t len = std::min(points.length(), vertex_count);
                                MFloatPoint *points_ptr = &points[0];
                                for (uint32_t pi = 0; pi < len; ++pi) {
                                    frame.points[pi] = to_float3(points_ptr[pi]) - dst.points[pi];
                                }
                            }
                        }
                        if (!handled) {
                            MPlug plug_points(plug_itip.child(3)); // .inputPointsTarget
                            MObject obj_points;
                            plug_points.getValue(obj_points);
                            if (!obj_points.isNull() && obj_points.hasFn(MFn::kPointArrayData)) {
                                handled = true;

                                MFnPointArrayData fn_points(obj_points);
                                uint32_t len = std::min(fn_points.length(), vertex_count);
                                MPoint *points_ptr(&fn_points[0]);
                                for (uint32_t pi = 0; pi < len; ++pi) {
                                    frame.points[pi] = to_float3(points_ptr[pi]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // get skinning data
    if (m_sync_bones && !fn_skin.object().isNull()) {
        // request bake TRS
        dst.refine_settings.flags.apply_local2world = 1;
        dst.refine_settings.local2world = dst.toMatrix();

        // get bone data
        MPlug plug_bindprematrix = fn_skin.findPlug("bindPreMatrix");
        MDagPathArray joint_paths;
        auto num_joints = fn_skin.influenceObjects(joint_paths);

        {
            std::unique_lock<std::mutex> lock(m_mutex_extract_mesh);
            for (uint32_t ij = 0; ij < num_joints; ij++) {
                auto joint = joint_paths[ij].node();
                notifyUpdateTransform(joint, true);

                auto bone = new ms::BoneData();
                bone->path = GetPath(joint);
                if (dst.bones.empty())
                    dst.root_bone = GetRootPath(joint);
                dst.bones.emplace_back(bone);

                MObject matrix_obj;
                auto ijoint = fn_skin.indexForInfluenceObject(joint_paths[ij], nullptr);
                plug_bindprematrix.elementByLogicalIndex(ijoint).getValue(matrix_obj);
                MMatrix bindpose = MFnMatrixData(matrix_obj).matrix();
                bone->bindpose.assign(bindpose[0]);
            }
        }

        // get weights
        MDagPath mesh_path = GetDagPath(mmesh.object());
        MItGeometry gi(mesh_path);
        while (!gi.isDone()) {
            MFloatArray weights;
            uint32_t influence_count;
            fn_skin.getWeights(mesh_path, gi.component(), weights, influence_count);

            for (uint32_t ij = 0; ij < influence_count; ij++) {
                dst.bones[ij]->weights.push_back(weights[ij]);
            }
            gi.next();
        }

        // handling tweaks
        if (m_apply_tweak) {

            // vertex tweak
            {
                MObject tweak;
                MObject skin_obj = fn_skin.object();
                MItDependencyGraph it(skin_obj, MFn::kTweak, MItDependencyGraph::kUpstream);
                if (!it.isDone()) {
                    tweak = it.currentItem();
                }
                if (!tweak.isNull()) {
                    MFnDependencyNode fn_tweak(tweak);
                    auto plug_vlist = fn_tweak.findPlug("vlist");
                    if (plug_vlist.isArray() && (int)plug_vlist.numElements() > skin_index) {
                        auto plug_vertex = plug_vlist.elementByPhysicalIndex(skin_index).child(0);
                        if (plug_vertex.isArray()) {
                            auto vertices_len = plug_vertex.numElements();
                            for (uint32_t vi = 0; vi < vertices_len; ++vi) {
                                MPlug p3 = plug_vertex.elementByPhysicalIndex(vi);
                                int li = p3.logicalIndex();
                                mu::float3 v;
                                p3.child(0).getValue(v.x);
                                p3.child(1).getValue(v.y);
                                p3.child(2).getValue(v.z);
                                dst.points[li] += v;
                            }
                        }
                    }
                }
            }

            // uv tweak
            {
                MObject tweak;
                MObject skin_obj = fn_skin.object();
                MItDependencyGraph it(skin_obj, MFn::kPolyTweakUV, MItDependencyGraph::kDownstream);
                if (!it.isDone()) {
                    tweak = it.currentItem();
                }
                if (!tweak.isNull()) {
                    MFnDependencyNode fn_tweak(tweak);
                    auto plug_uvsetname = fn_tweak.findPlug("uvSetName");
                    auto plug_uv = fn_tweak.findPlug("uvTweak");
                    if (plug_uv.isArray() && (int)plug_uv.numElements() > skin_index) {
                        auto plug_vertex = plug_uv.elementByPhysicalIndex(skin_index).child(0);
                        if (plug_vertex.isArray()) {
                            auto vertices_len = plug_vertex.numElements();
                            for (uint32_t vi = 0; vi < vertices_len; ++vi) {
                                MPlug p2 = plug_vertex.elementByPhysicalIndex(vi);
                                int li = p2.logicalIndex();
                                mu::float2 v;
                                p2.child(0).getValue(v.x);
                                p2.child(1).getValue(v.y);
                                dst.uv0[li] += v;
                            }
                        }
                    }
                }
            }
        }
    }

    dst.setupFlags();
    return true;
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
