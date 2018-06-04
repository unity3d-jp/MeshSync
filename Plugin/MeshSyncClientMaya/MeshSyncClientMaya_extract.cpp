#define _MApiVersion
#include "pch.h"
#include "MayaUtils.h"
#include "MeshSyncClientMaya.h"
#include "Commands.h"


void MeshSyncClientMaya::exportMaterials()
{
    m_material_id_table.clear();

    MItDependencyNodes it(MFn::kLambert);
    while (!it.isDone()) {
        MFnLambertShader fn(it.item());

        auto tmp = ms::Material::create();
        tmp->name = fn.name().asChar();
        tmp->color = to_float4(fn.color());
        tmp->id = (int)m_material_id_table.size();
        m_material_id_table.push_back(fn.name());
        m_materials.push_back(tmp);

        it.next();
    }
}

bool MeshSyncClientMaya::exportObject(TreeNode *n, bool force)
{
    if (!n || n->dst_obj)
        return false;

    auto& trans = n->trans->node;
    auto& shape = n->shape->node;

    ms::TransformPtr ret;
    if (m_settings.sync_meshes && shape.hasFn(MFn::kMesh)) {
        exportObject(n->parent, true);
        auto dst = ms::Mesh::create();
        extractMeshData(*dst, n);
        m_meshes.emplace_back(dst);
        ret = dst;
    }
    else if (m_settings.sync_cameras &&shape.hasFn(MFn::kCamera)) {
        exportObject(n->parent, true);
        auto dst = ms::Camera::create();
        extractCameraData(*dst, n);
        m_objects.emplace_back(dst);
        ret = dst;
    }
    else if (m_settings.sync_lights &&shape.hasFn(MFn::kLight)) {
        exportObject(n->parent, true);
        auto dst = ms::Light::create();
        extractLightData(*dst, n);
        m_objects.emplace_back(dst);
        ret = dst;
    }
    else if ((m_settings.sync_bones && shape.hasFn(MFn::kJoint)) || force) {
        exportObject(n->parent, true);
        auto dst = ms::Transform::create();
        extractTransformData(*dst, n);
        m_objects.emplace_back(dst);
        ret = dst;
    }

    if (ret) {
        ret->path = n->path;
        ret->index = n->index;
        n->dst_obj = ret.get();
        return true;
    }
    else {
        return false;
    }
}

static void ExtractTransformData(TreeNode *n, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
{
    if (n->trans->isInstance()) {
        n = n->getPrimaryInstanceNode();
    }

    // get TRS from world matrix.
    // note: world matrix is a result of local TRS + parent TRS + constraints.
    //       handling constraints by ourselves is extremely difficult. so getting TRS from world matrix is most reliable and easy way.

    auto mat = mu::float4x4::identity();
    {
        auto& trans = n->trans->node;
        MObject obj_wmat;
        MFnTransform(trans).findPlug("worldMatrix").elementByLogicalIndex(0).getValue(obj_wmat);
        mat = to_float4x4(MFnMatrixData(obj_wmat).matrix());
        vis = IsVisible(trans);
    }

    // get inverse parent matrix to calculate local matrix.
    // note: using parentInverseMatrix plug seems more appropriate, but it seems sometimes have incorrect value on Maya2016...
    if (n->parent) {
        auto& parent = n->parent->trans->node;
        MObject obj_pwmat;
        MFnTransform(parent).findPlug("worldMatrix").elementByLogicalIndex(0).getValue(obj_pwmat);
        auto pwmat = to_float4x4(MFnMatrixData(obj_pwmat).matrix());
        mat *= mu::invert(pwmat);
    }

    pos = extract_position(mat);
    rot = extract_rotation(mat);
    scale = extract_scale(mat);
}

static void ExtractCameraData(TreeNode *n, bool& ortho, float& near_plane, float& far_plane, float& fov,
    float& horizontal_aperture, float& vertical_aperture, float& focal_length, float& focus_distance)
{
    auto& shape = n->shape->node;
    MFnCamera mcam(shape);

    ortho = mcam.isOrtho();
    near_plane = (float)mcam.nearClippingPlane();
    far_plane = (float)mcam.farClippingPlane();
    fov = (float)mcam.verticalFieldOfView() * ms::Rad2Deg;

    horizontal_aperture = (float)mcam.horizontalFilmAperture() * InchToMillimeter;
    vertical_aperture = (float)mcam.verticalFilmAperture() * InchToMillimeter;
    focal_length = (float)mcam.focalLength();
    focus_distance = (float)mcam.focusDistance();
}

static void ExtractLightData(TreeNode *n, ms::Light::LightType& type, mu::float4& color, float& intensity, float& spot_angle)
{
    auto& shape = n->shape->node;
    if (shape.hasFn(MFn::kSpotLight)) {
        MFnSpotLight mlight(shape);
        type = ms::Light::LightType::Spot;
        spot_angle = (float)mlight.coneAngle() * mu::Rad2Deg;
    }
    else if (shape.hasFn(MFn::kDirectionalLight)) {
        MFnDirectionalLight mlight(shape);
        type = ms::Light::LightType::Directional;
    }
    else if (shape.hasFn(MFn::kPointLight)) {
        MFnPointLight mlight(shape);
        type = ms::Light::LightType::Point;
    }
    else if (shape.hasFn(MFn::kAreaLight)) {
        MFnAreaLight mlight(shape);
        type = ms::Light::LightType::Area;
    }

    MFnLight mlight(shape);
    auto mcol = mlight.color();
    color = { mcol.r, mcol.g, mcol.b, mcol.a };
    intensity = mlight.intensity();
}

void MeshSyncClientMaya::extractTransformData(ms::Transform& dst, TreeNode *n)
{
    ExtractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible_hierarchy);
}

void MeshSyncClientMaya::extractCameraData(ms::Camera& dst, TreeNode *n)
{
    ExtractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible_hierarchy);
    dst.rotation = mu::flipY(dst.rotation);

    ExtractCameraData(n, dst.is_ortho, dst.near_plane, dst.far_plane, dst.fov,
        dst.horizontal_aperture, dst.vertical_aperture, dst.focal_length, dst.focus_distance);

}

void MeshSyncClientMaya::extractLightData(ms::Light& dst, TreeNode *n)
{
    ExtractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible_hierarchy);
    dst.rotation = mu::flipY(dst.rotation);

    ExtractLightData(n, dst.light_type, dst.color, dst.intensity, dst.spot_angle);
}

void MeshSyncClientMaya::extractMeshData(ms::Mesh& dst, TreeNode *n)
{
    if (m_material_id_table.empty()) {
        exportMaterials();
    }

    ExtractTransformData(n, dst.position, dst.rotation, dst.scale, dst.visible_hierarchy);

    auto task = [this, &dst, n]() {
        doExtractMeshData(dst, n);
    };
    m_extract_tasks[n->shape->branches.front()].add(n, task);
}

void MeshSyncClientMaya::doExtractMeshData(ms::Mesh& dst, TreeNode *n)
{
    auto& trans = n->trans->node;
    auto& shape = n->shape->node;

    if (!shape.hasFn(MFn::kMesh)) { return; }

    dst.visible = IsVisible(shape);
    if (!dst.visible) { return; }

    MFnMesh mmesh(shape);
    MFnSkinCluster fn_skin(FindSkinCluster(mmesh.object()));
    bool is_skinned = !fn_skin.object().isNull();

    if (n->isInstance()) {
        auto primary = n->getPrimaryInstanceNode();
        if (n != primary) {
            dst.reference = primary->path;
            return;
        }
    }

    dst.flags.has_refine_settings = 1;
    dst.flags.apply_trs = 1;
    dst.refine_settings.flags.gen_tangents = 1;
    dst.refine_settings.flags.swap_faces = 1;

    if (!mmesh.object().hasFn(MFn::kMesh)) {
        // return empty mesh
        return;
    }

    MFnMesh fn_src_mesh(mmesh.object());
    MFnBlendShapeDeformer fn_blendshape(FindBlendShape(mmesh.object()));
    int skin_index = 0;

    // if target has skinning or blendshape, use pre-deformed mesh as source.
    // * this code assumes blendshape is applied always after skinning, and there is no multiple blendshapes or skinnings.
    // * maybe this cause a problem..
    if (m_settings.sync_blendshapes && !fn_blendshape.object().isNull()) {
        auto orig_mesh = FindOrigMesh(trans);
        if (orig_mesh.hasFn(MFn::kMesh)) {
            fn_src_mesh.setObject(orig_mesh);
        }
    }
    if (m_settings.sync_bones && is_skinned) {
        auto orig_mesh = FindOrigMesh(trans);
        if (orig_mesh.hasFn(MFn::kMesh)) {
            fn_src_mesh.setObject(orig_mesh);
            skin_index = fn_skin.indexForOutputShape(mmesh.object());
        }
    }

    // get points
    {
        MFloatPointArray points;
        if (fn_src_mesh.getPoints(points) != MStatus::kSuccess) {
            // return empty mesh
            return;
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
    if (m_settings.sync_normals) {
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
    if (m_settings.sync_uvs) {
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
    if (m_settings.sync_colors) {
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
                mids[si] = getMaterialID(lambert.name());
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



    auto apply_tweak = [&dst](MObject deformer, int obj_index) {
        MItDependencyGraph it(deformer, MFn::kTweak, MItDependencyGraph::kUpstream);
        if (!it.isDone()) {
            MObject tweak = it.currentItem();
            if (!tweak.isNull()) {
                MFnDependencyNode fn_tweak(tweak);
                auto plug_vlist = fn_tweak.findPlug("vlist");
                if (plug_vlist.isArray() && (int)plug_vlist.numElements() > obj_index) {
                    auto plug_vertex = plug_vlist.elementByPhysicalIndex(obj_index).child(0);
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
    };

    auto apply_uv_tweak = [&dst](MObject deformer, int obj_index) {
        MItDependencyGraph it(deformer, MFn::kPolyTweakUV, MItDependencyGraph::kDownstream);
        if (!it.isDone()) {
            MObject tweak = it.currentItem();
            if (!tweak.isNull()) {
                MFnDependencyNode fn_tweak(tweak);
                auto plug_uvsetname = fn_tweak.findPlug("uvSetName");
                auto plug_uv = fn_tweak.findPlug("uvTweak");
                if (plug_uv.isArray() && (int)plug_uv.numElements() > obj_index) {
                    auto plug_vertex = plug_uv.elementByPhysicalIndex(obj_index).child(0);
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
    };


    // get blendshape data
    if (m_settings.sync_blendshapes && !fn_blendshape.object().isNull()) {
        // https://knowledge.autodesk.com/search-result/caas/CloudHelp/cloudhelp/2018/ENU/Maya-Tech-Docs/Nodes/blendShape-html.html

        auto gen_delta = [&dst](ms::BlendShapeData::Frame& dst_frame, MPlug plug_geom) {
            MObject obj_geom;
            plug_geom.getValue(obj_geom);
            if (!obj_geom.isNull() && obj_geom.hasFn(MFn::kMesh)) {

                MFnMesh fn_geom(obj_geom);
                MFloatPointArray points;
                fn_geom.getPoints(points);

                uint32_t len = std::min(points.length(), (uint32_t)dst.points.size());
                MFloatPoint *points_ptr = &points[0];
                for (uint32_t pi = 0; pi < len; ++pi) {
                    dst_frame.points[pi] = to_float3(points_ptr[pi]) - dst.points[pi];
                }
            }
        };

        auto retrieve_delta = [&dst](ms::BlendShapeData::Frame& dst_frame, MPlug plug_ipt, MPlug plug_ict) {
            MObject obj_component_list;
            MObject obj_points;
            {
                MObject obj_cld;
                plug_ict.getValue(obj_cld);
                if (!obj_cld.isNull() && obj_cld.hasFn(MFn::kComponentListData)) {
                    MFnComponentListData fn_cld(obj_cld);
                    uint32_t len = fn_cld.length();
                    for (uint32_t ci = 0; ci < len; ++ci) {
                        MObject tmp = fn_cld[ci];
                        if (tmp.apiType() == MFn::kMeshVertComponent) {
                            obj_component_list = tmp;
                            break;
                        }
                    }
                }
            }
            {
                MObject tmp;
                plug_ipt.getValue(tmp);
                if (!tmp.isNull() && tmp.hasFn(MFn::kPointArrayData)) {
                    obj_points = tmp;
                }
            }
            if (!obj_component_list.isNull() && !obj_points.isNull()) {
                MIntArray indices;
                MFnSingleIndexedComponent fn_indices(obj_component_list);
                fn_indices.getElements(indices);
                int *indices_ptr = &indices[0];

                MFnPointArrayData fn_points(obj_points);
                MPoint *points_ptr = &fn_points[0];

                uint32_t len = std::min(fn_points.length(), (uint32_t)dst.points.size());
                for (uint32_t pi = 0; pi < len; ++pi) {
                    dst_frame.points[indices_ptr[pi]] = to_float3(points_ptr[pi]);
                }
            }
        };

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
                    auto dst_bs = ms::BlendShapeData::create();
                    dst.blendshapes.push_back(dst_bs);
                    MPlug plug_wc = plug_weight.elementByPhysicalIndex(idx_itg);
                    dst_bs->name = plug_wc.name().asChar();
                    plug_wc.getValue(dst_bs->weight);
                    dst_bs->weight *= 100.0f; // 0.0f-1.0f -> 0.0f-100.0f

                    MPlug plug_itgp(plug_itg.elementByPhysicalIndex(idx_itg));
                    uint32_t delta_index = plug_itgp.logicalIndex();

                    MPlug plug_iti(plug_itgp.child(0)); // .inputTarget[idx_it].inputTargetGroup[idx_itg].inputTargetItem
                    uint32_t num_iti(plug_iti.evaluateNumElements());
                    for (uint32_t idx_iti = 0; idx_iti != num_iti; ++idx_iti) {
                        MPlug plug_itip(plug_iti.elementByPhysicalIndex(idx_iti));

                        dst_bs->frames.push_back(ms::BlendShapeData::Frame());
                        auto& dst_frame = dst_bs->frames.back();
                        dst_frame.weight = float(plug_itip.logicalIndex() - 5000) / 10.0f; // index 5000-6000 -> weight 0.0f-100.0f
                        dst_frame.points.resize_zeroclear(dst.points.size());

                        MPlug plug_geom(plug_itip.child(0)); // .inputGeomTarget
                        if (plug_geom.isConnected()) {
                            // in this case target is geometry
                            gen_delta(dst_frame, plug_geom);
                        }
                        else {
                            // in this case there is no geometry target. try to retrieves deltas
                            MPlug plug_ipt(plug_itip.child(3)); // .inputPointsTarget
                            MPlug plug_ict(plug_itip.child(4)); // .inputComponentsTarget
                            retrieve_delta(dst_frame, plug_ipt, plug_ict);
                        }
                    }
                }
            }
        }
    }

    // get skinning data
    if (m_settings.sync_bones && !fn_skin.object().isNull()) {
        // request bake TRS
        dst.refine_settings.flags.apply_local2world = 1;
        dst.refine_settings.local2world = dst.toMatrix();

        // get bone data
        MPlug plug_bindprematrix = fn_skin.findPlug("bindPreMatrix");
        MDagPathArray joint_paths;
        auto num_joints = fn_skin.influenceObjects(joint_paths);

        for (uint32_t ij = 0; ij < num_joints; ij++) {
            auto joint = joint_paths[ij].node();

            auto bone = ms::BoneData::create();
            bone->path = GetPath(joint);
            if (dst.bones.empty())
                dst.root_bone = GetRootBonePath(joint);
            dst.bones.push_back(bone);

            MObject matrix_obj;
            auto ijoint = fn_skin.indexForInfluenceObject(joint_paths[ij], nullptr);
            plug_bindprematrix.elementByLogicalIndex(ijoint).getValue(matrix_obj);
            bone->bindpose = to_float4x4(MFnMatrixData(matrix_obj).matrix());
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
    }

    // apply tweaks
    if (m_settings.apply_tweak) {
        apply_tweak(shape, skin_index);
        apply_uv_tweak(shape, skin_index);
    }

    dst.setupFlags();
}


void MeshSyncClientMaya::AnimationRecord::operator()(MeshSyncClientMaya *_this)
{
    (_this->*extractor)(*dst, tn);
}


int MeshSyncClientMaya::exportAnimations(SendScope scope)
{
    // create default clip
    m_animations.push_back(ms::AnimationClip::create());

    int num_exported = 0;
    auto export_branches = [&](DAGNode& rec) {
        for (auto *tn : rec.branches) {
            if (exportAnimation(tn))
                ++num_exported;
        }
    };


    // gather target data
    if (scope == SendScope::Selected) {
        MSelectionList list;
        MGlobal::getActiveSelectionList(list);
        for (uint32_t i = 0; i < list.length(); i++) {
            MObject node;
            list.getDependNode(i, node);
            export_branches(m_dag_nodes[node]);
        }
    }
    else { // all
        auto handler = [&](MObject& node) {
            export_branches(m_dag_nodes[node]);
        };
        EnumerateNode(MFn::kJoint, handler);
        EnumerateNode(MFn::kCamera, handler);
        EnumerateNode(MFn::kLight, handler);
        EnumerateNode(MFn::kMesh, handler);
    }

    // extract
    auto time_current = MAnimControl::currentTime();
    auto time_begin = MAnimControl::minTime();
    auto time_end = MAnimControl::maxTime();
    auto interval = MTime(1.0 / m_settings.animation_sps, MTime::kSeconds);

    int reserve_size = int((time_end.as(MTime::kSeconds) - time_begin.as(MTime::kSeconds)) / interval.as(MTime::kSeconds)) + 1;
    for (auto& kvp : m_anim_records) {
        kvp.second.dst->reserve(reserve_size);
    };

    m_ignore_update = true;
    for (MTime t = time_begin; t <= time_end; t += interval) {
        // advance frame and record
        m_anim_time = (float)t.as(MTime::kSeconds);
        MGlobal::viewFrame(t);
        for (auto& kvp : m_anim_records)
            kvp.second(this);
    }
    MGlobal::viewFrame(time_current);
    m_ignore_update = false;

    // cleanup
    m_anim_records.clear();

    // reduction
    for (auto& clip : m_animations)
        clip->reduction();

    // erase empty animation
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(), [](ms::AnimationClipPtr& p) { return p->empty(); }),
        m_animations.end());

    if (num_exported == 0)
        m_animations.clear();

    return num_exported;
}

bool MeshSyncClientMaya::exportAnimation(TreeNode *n)
{
    if (!n || n->dst_anim)
        return false;

    auto& trans = n->trans->node;
    auto& shape = n->shape->node;

    ms::AnimationPtr dst;
    AnimationRecord::extractor_t extractor = nullptr;

    if (MAnimUtil::isAnimated(trans) || MAnimUtil::isAnimated(shape)) {
        if (shape.hasFn(MFn::kCamera)) {
            exportAnimation(n->parent);
            dst = ms::CameraAnimation::create();
            extractor = &MeshSyncClientMaya::extractCameraAnimationData;
        }
        else if (shape.hasFn(MFn::kLight)) {
            exportAnimation(n->parent);
            dst = ms::LightAnimation::create();
            extractor = &MeshSyncClientMaya::extractLightAnimationData;
        }
        else if (shape.hasFn(MFn::kMesh)) {
            exportAnimation(n->parent);
            dst = ms::MeshAnimation::create();
            extractor = &MeshSyncClientMaya::extractMeshAnimationData;
        }
        else {
            exportAnimation(n->parent);
            dst = ms::TransformAnimation::create();
            extractor = &MeshSyncClientMaya::extractTransformAnimationData;
        }
    }

    if (dst) {
        dst->path = n->path;
        auto& rec = m_anim_records[n];
        rec.tn = n;
        rec.dst = dst.get();
        rec.extractor = extractor;
        n->dst_anim = dst.get();
        m_animations.front()->animations.emplace_back(dst);
        return true;
    }
    else {
        return false;
    }
}

void MeshSyncClientMaya::extractTransformAnimationData(ms::Animation& dst_, TreeNode *n)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    auto pos = mu::float3::zero();
    auto rot = mu::quatf::identity();
    auto scale = mu::float3::one();
    bool vis = true;
    ExtractTransformData(n, pos, rot, scale, vis);

    float t = m_anim_time * m_settings.animation_time_scale;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
    //dst.visible.push_back({ t, vis });
}

void MeshSyncClientMaya::extractCameraAnimationData(ms::Animation& dst_, TreeNode *n)
{
    extractTransformAnimationData(dst_, n);

    auto& dst = (ms::CameraAnimation&)dst_;
    {
        auto& last = dst.rotation.back();
        last.value = mu::flipY(last.value);
    }

    bool ortho;
    float near_plane, far_plane, fov, horizontal_aperture, vertical_aperture, focal_length, focus_distance;
    ExtractCameraData(n, ortho, near_plane, far_plane, fov, horizontal_aperture, vertical_aperture, focal_length, focus_distance);

    float t = m_anim_time * m_settings.animation_time_scale;
    dst.near_plane.push_back({ t , near_plane });
    dst.far_plane.push_back({ t , far_plane });
    dst.fov.push_back({ t , fov });

    // params for physical camera. not needed for now.
#if 0
    dst.horizontal_aperture.push_back({ t , horizontal_aperture });
    dst.vertical_aperture.push_back({ t , vertical_aperture });
    dst.focal_length.push_back({ t , focal_length });
    dst.focus_distance.push_back({ t , focus_distance });
#endif
}

void MeshSyncClientMaya::extractLightAnimationData(ms::Animation& dst_, TreeNode *n)
{
    extractTransformAnimationData(dst_, n);

    auto& dst = (ms::LightAnimation&)dst_;
    {
        auto& last = dst.rotation.back();
        last.value = mu::flipY(last.value);
    }

    ms::Light::LightType type;
    mu::float4 color;
    float intensity;
    float spot_angle;
    ExtractLightData(n, type, color, intensity, spot_angle);

    float t = m_anim_time * m_settings.animation_time_scale;
    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });

    auto& shape = n->shape->node;
    if (shape.hasFn(MFn::kSpotLight)) {
        dst.spot_angle.push_back({ t, spot_angle });
    }
}

void MeshSyncClientMaya::extractMeshAnimationData(ms::Animation & dst_, TreeNode *n)
{
    extractTransformAnimationData(dst_, n);

    auto& dst = (ms::MeshAnimation&)dst_;
    auto& shape = n->shape->node;

    float t = m_anim_time * m_settings.animation_time_scale;

    // get blendshape weights
    MFnBlendShapeDeformer fn_blendshape(FindBlendShape(shape));
    if (!fn_blendshape.object().isNull()) {
        MPlug plug_weight = fn_blendshape.findPlug("weight");
        MPlug plug_it = fn_blendshape.findPlug("inputTarget");
        uint32_t num_it = plug_it.evaluateNumElements();
        for (uint32_t idx_it = 0; idx_it < num_it; ++idx_it) {
            MPlug plug_itp(plug_it.elementByPhysicalIndex(idx_it));
            if (plug_itp.logicalIndex() == 0) {
                MPlug plug_itg(plug_itp.child(0)); // .inputTarget[idx_it].inputTargetGroup
                uint32_t num_itg = plug_itg.evaluateNumElements();

                for (uint32_t idx_itg = 0; idx_itg < num_itg; ++idx_itg) {
                    MPlug plug_wc = plug_weight.elementByPhysicalIndex(idx_itg);
                    std::string name = plug_wc.name().asChar();

                    auto bsa = dst.findOrCreateBlendshapeAnimation(name.c_str());
                    float weight = 0.0f;
                    plug_wc.getValue(weight);
                    bsa->weight.push_back({ t, weight * 100.0f });
                }
            }
        }
    }
}


void MeshSyncClientMaya::exportConstraint(TreeNode *n)
{
    auto& trans = n->trans->node;
    auto& shape = n->shape->node;

    EachConstraints(trans, [&](const MObject& constraint) {
        if (constraint.hasFn(MFn::kAimConstraint)) {
            auto dst = ms::AimConstraint::create();
            m_constraints.push_back(dst);
            extractConstraintData(*dst, n);
        }
        else if (constraint.hasFn(MFn::kParentConstraint)) {
            auto dst = ms::ParentConstraint::create();
            m_constraints.push_back(dst);
            extractConstraintData(*dst, n);
        }
        else if (constraint.hasFn(MFn::kPointConstraint)) {
            auto dst = ms::PositionConstraint::create();
            m_constraints.push_back(dst);
            extractConstraintData(*dst, n);
        }
        else if (constraint.hasFn(MFn::kScaleConstraint)) {
            auto dst = ms::ScaleConstraint::create();
            m_constraints.push_back(dst);
            extractConstraintData(*dst, n);
        }
        else {
            // not supported
        }
    });
}

void MeshSyncClientMaya::extractConstraintData(ms::Constraint& dst, TreeNode *n)
{
    // todo
    // maybe I give up to support this..
}

