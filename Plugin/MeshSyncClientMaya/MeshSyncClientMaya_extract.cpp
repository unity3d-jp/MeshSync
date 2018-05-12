#define _MApiVersion
#include "pch.h"
#include "MayaUtils.h"
#include "MeshSyncClientMaya.h"
#include "Commands.h"

void MeshSyncClientMaya::addAnimation(ms::Animation *anim)
{
    lock_t l(m_mutex);
    m_client_animations.emplace_back(anim);
}

void MeshSyncClientMaya::exportMaterials()
{
    m_material_id_table.clear();

    MItDependencyNodes it(MFn::kLambert);
    while (!it.isDone()) {
        MFnLambertShader fn(it.item());

        auto tmp = new ms::Material();
        tmp->name = fn.name().asChar();
        tmp->color = to_float4(fn.color());
        tmp->id = (int)m_material_id_table.size();
        m_material_id_table.push_back(fn.uuid());
        m_client_materials.emplace_back(tmp);

        it.next();
    }
}

void MeshSyncClientMaya::extractTransformData(ms::Transform& dst, MObject src)
{
    if (m_settings.sync_constraints) {
        EachConstraints(src, [this, &src](const MObject& constraint) {
            if (constraint.hasFn(MFn::kAimConstraint)) {
                auto dst = new ms::AimConstraint();
                m_client_constraints.emplace_back(dst);
                extractConstraintData(*dst, constraint, src);
            }
            else if (constraint.hasFn(MFn::kParentConstraint)) {
                auto dst = new ms::ParentConstraint();
                m_client_constraints.emplace_back(dst);
                extractConstraintData(*dst, constraint, src);
            }
            else if (constraint.hasFn(MFn::kPointConstraint)) {
                auto dst = new ms::PositionConstraint();
                m_client_constraints.emplace_back(dst);
                extractConstraintData(*dst, constraint, src);
            }
            else if (constraint.hasFn(MFn::kScaleConstraint)) {
                auto dst = new ms::ScaleConstraint();
                m_client_constraints.emplace_back(dst);
                extractConstraintData(*dst, constraint, src);
            }
        });
    }

    auto task = [this, &dst, src]() {
        doExtractTransformData(dst, src);
    };
    m_extract_tasks.push_back(task);
}

// GetValue: [](MPlug src, MObject& dst)
static inline void ExtractTransformData(MObject src, mu::float3& pos, mu::quatf& rot, mu::float3& scale, bool& vis)
{
    MFnTransform mtrans(src);

    vis = IsVisible(src);

    auto plug_wmat = mtrans.findPlug("worldMatrix");
    if (!plug_wmat.isNull()) {
        MObject obj_wmat;
        plug_wmat.elementByLogicalIndex(0).getValue(obj_wmat);
        auto mat = to_float4x4(MFnMatrixData(obj_wmat).matrix());

        auto plug_pimat = mtrans.findPlug("parentInverseMatrix");
        if (!plug_pimat.isNull()) {
            MObject obj_pimat;
            plug_pimat.elementByLogicalIndex(0).getValue(obj_pimat);
            auto pimat = to_float4x4(MFnMatrixData(obj_pimat).matrix());
            mat *= pimat;
        }

        pos = extract_position(mat);
        rot = extract_rotation(mat);
        scale = extract_scale(mat);
    }
}

void MeshSyncClientMaya::doExtractTransformData(ms::Transform & dst, MObject src)
{
    dst.path = GetPath(src);
    ExtractTransformData(src, dst.position, dst.rotation, dst.scale, dst.visible_hierarchy);
}


void MeshSyncClientMaya::extractCameraData(ms::Camera& dst, MObject src)
{
    auto task = [this, &dst, src]() {
        doExtractCameraData(dst, src);
    };
    m_extract_tasks.push_back(task);
}

static void ExtractCameraData(MObject shape, float& near_plane, float& far_plane, float& fov,
    float& horizontal_aperture, float& vertical_aperture, float& focal_length, float& focus_distance)
{
    MFnCamera mcam(shape);

    near_plane = (float)mcam.nearClippingPlane();
    far_plane = (float)mcam.farClippingPlane();
    fov = (float)mcam.horizontalFieldOfView() * ms::Rad2Deg * 0.5f;

    horizontal_aperture = (float)mcam.horizontalFilmAperture() * InchToMillimeter;
    vertical_aperture = (float)mcam.verticalFilmAperture() * InchToMillimeter;
    focal_length = (float)mcam.focalLength();
    focus_distance = (float)mcam.focusDistance();
}

void MeshSyncClientMaya::doExtractCameraData(ms::Camera & dst, MObject src)
{
    doExtractTransformData(dst, src);
    dst.rotation = mu::flipY(dst.rotation);

    auto shape = GetShape(src);
    if (!shape.hasFn(MFn::kCamera)) {
        return;
    }

    MFnCamera mcam(shape);
    dst.is_ortho = mcam.isOrtho();
    ExtractCameraData(shape, dst.near_plane, dst.far_plane, dst.fov,
        dst.horizontal_aperture, dst.vertical_aperture, dst.focal_length, dst.focus_distance);
}

void MeshSyncClientMaya::extractLightData(ms::Light& dst, MObject src)
{
    auto task = [this, &dst, src]() {
        doExtractLightData(dst, src);
    };
    m_extract_tasks.push_back(task);
}

static void ExtractLightData(MObject shape, mu::float4& color, float& intensity, float& spot_angle)
{
    if (shape.hasFn(MFn::kSpotLight)) {
        MFnSpotLight mlight(shape);
        spot_angle = (float)mlight.coneAngle() * mu::Rad2Deg;
    }

    MFnLight mlight(shape);
    auto mcol = mlight.color();
    color = { mcol.r, mcol.g, mcol.b, mcol.a };
    intensity = mlight.intensity();
}

void MeshSyncClientMaya::doExtractLightData(ms::Light & dst, MObject src)
{
    doExtractTransformData(dst, src);
    dst.rotation = mu::flipY(dst.rotation);

    auto shape = GetShape(src);
    if (shape.hasFn(MFn::kSpotLight)) {
        MFnSpotLight mlight(shape);
        dst.light_type = ms::Light::LightType::Spot;
    }
    else if (shape.hasFn(MFn::kDirectionalLight)) {
        MFnDirectionalLight mlight(shape);
        dst.light_type = ms::Light::LightType::Directional;
    }
    else if (shape.hasFn(MFn::kPointLight)) {
        MFnPointLight mlight(shape);
        dst.light_type = ms::Light::LightType::Point;
    }
    else if (shape.hasFn(MFn::kAreaLight)) {
        MFnAreaLight mlight(shape);
        dst.light_type = ms::Light::LightType::Area;
    }
    else {
        return;
    }

    ExtractLightData(shape, dst.color, dst.intensity, dst.spot_angle);
}


void MeshSyncClientMaya::extractMeshData(ms::Mesh& dst, MObject src)
{
    if (m_material_id_table.empty()) {
        exportMaterials();
    }

    auto task = [this, &dst, src]() {
        doExtractMeshData(dst, src);
    };
    m_extract_tasks.push_back(task);
}

void MeshSyncClientMaya::doExtractMeshData(ms::Mesh& dst, MObject src)
{
    doExtractTransformData(dst, src);

    auto shape = GetShape(src);
    if (!shape.hasFn(MFn::kMesh)) { return; }

    dst.visible = IsVisible(shape);
    if (!dst.visible) { return; }

    MFnMesh mmesh(shape);

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
    MFnSkinCluster fn_skin(FindSkinCluster(mmesh.object()));
    int skin_index = 0;

    // if target has skinning or blendshape, use pre-deformed mesh as source.
    // * this code assumes blendshape is applied always after skinning, and there is no multiple blendshapes or skinnings.
    // * maybe this cause a problem..
    if (m_settings.sync_blendshapes && !fn_blendshape.object().isNull()) {
        auto orig_mesh = FindOrigMesh(src);
        if (orig_mesh.hasFn(MFn::kMesh)) {
            fn_src_mesh.setObject(orig_mesh);
        }
    }
    if (m_settings.sync_bones && !fn_skin.object().isNull()) {
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
                    auto dst_bs = new ms::BlendShapeData();
                    dst.blendshapes.emplace_back(dst_bs);
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

        // apply tweaks
        if (m_settings.apply_tweak) {
            apply_tweak(fn_blendshape.object(), skin_index);
            apply_uv_tweak(fn_blendshape.object(), skin_index);
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

            auto bone = new ms::BoneData();
            bone->path = GetPath(joint);
            if (dst.bones.empty())
                dst.root_bone = GetRootBonePath(joint);
            dst.bones.emplace_back(bone);

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

        // apply tweaks
        if (m_settings.apply_tweak) {
            apply_tweak(fn_blendshape.object(), skin_index);
            apply_uv_tweak(fn_blendshape.object(), skin_index);
        }
    }

    dst.setupFlags();
}


void MeshSyncClientMaya::extractConstraintData(ms::Constraint& dst, MObject src, MObject node)
{
    dst.path = GetPath(node);

    auto task = [this, &dst, src, node]() {
        doExtractConstraintData(dst, src, node);
    };
    m_extract_tasks.push_back(task);
}
void MeshSyncClientMaya::doExtractConstraintData(ms::Constraint& dst, MObject src, MObject node)
{
}

int MeshSyncClientMaya::exportAnimations(SendScope scope)
{
    // gather target data
    if (scope == SendScope::Selected) {
        MSelectionList list;
        MGlobal::getActiveSelectionList(list);
        for (uint32_t i = 0; i < list.length(); i++) {
            MObject node;
            list.getDependNode(i, node);
            exportAnimation(node, GetShape(node));
        }
    }
    else {
        auto exportNode = [this](MObject& node) {
            exportAnimation(node, MObject());
        };
        auto exportShape = [this](MObject& shape) {
            exportAnimation(GetTransform(shape), shape);
        };
        Enumerate(MFn::kJoint, exportNode);
        Enumerate(MFn::kCamera, exportShape);
        Enumerate(MFn::kLight, exportShape);
        Enumerate(MFn::kMesh, exportShape);
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
        m_current_time = (float)t.as(MTime::kSeconds);
        MGlobal::viewFrame(t);

        mu::parallel_for_each(m_anim_records.begin(), m_anim_records.end(), [this](AnimationRecords::value_type& kvp) {
            kvp.second(this);
        });
    }
    MGlobal::viewFrame(time_current);
    m_ignore_update = false;

    // cleanup
    int ret = (int)m_anim_records.size();
    m_anim_records.clear();

    // reduction
    mu::parallel_for_each(m_client_animations.begin(), m_client_animations.end(), [](ms::AnimationPtr& p) {
        p->reduction();
    });

    // erase empty animation
    m_client_animations.erase(
        std::remove_if(m_client_animations.begin(), m_client_animations.end(), [](ms::AnimationPtr& p) { return p->empty(); }),
        m_client_animations.end());
    return ret;
}

void MeshSyncClientMaya::exportAnimation(MObject node, MObject shape)
{
    if (node.isNull() || node.hasFn(MFn::kWorld))
        return;
    if (m_anim_records.find((void*&)node) != m_anim_records.end())
        return;

    AnimationRecord rec;
    {
        auto parent = GetParent(node);
        if (!parent.isNull()) {
            exportAnimation(parent, GetShape(parent));
        }
    }

    bool animated = MAnimUtil::isAnimated(node) || MAnimUtil::isAnimated(shape);
    if (m_settings.sync_cameras && shape.hasFn(MFn::kCamera) && animated) {
        rec.dst = new ms::CameraAnimation();
        rec.extractor = &MeshSyncClientMaya::extractCameraAnimationData;
    }
    else if (m_settings.sync_lights && shape.hasFn(MFn::kLight) && animated) {
        rec.dst = new ms::LightAnimation();
        rec.extractor = &MeshSyncClientMaya::extractLightAnimationData;
    }
    else if(animated) {
        rec.dst = new ms::TransformAnimation();
        rec.extractor = &MeshSyncClientMaya::extractTransformAnimationData;
    }

    if (rec.dst) {
        m_client_animations.emplace_back(rec.dst);
        rec.dst->path = GetPath(node);
        rec.node = node;
        rec.shape = shape;
        m_anim_records[(void*&)node] = std::move(rec);
    }
}

void MeshSyncClientMaya::extractTransformAnimationData(ms::Animation& dst_, MObject node, MObject /*shape*/)
{
    auto& dst = (ms::TransformAnimation&)dst_;

    auto pos = mu::float3::zero();
    auto rot = mu::quatf::identity();
    auto scale = mu::float3::one();
    bool vis = true;
    ExtractTransformData(node, pos, rot, scale, vis);

    float t = m_current_time * m_settings.animation_time_scale;
    dst.translation.push_back({ t, pos });
    dst.rotation.push_back({ t, rot });
    dst.scale.push_back({ t, scale });
    //dst.visible.push_back({ t, vis });
}

void MeshSyncClientMaya::extractCameraAnimationData(ms::Animation& dst_, MObject node, MObject shape)
{
    extractTransformAnimationData(dst_, node, shape);

    auto& dst = (ms::CameraAnimation&)dst_;
    {
        auto& last = dst.rotation.back();
        last.value = mu::flipY(last.value);
    }

    float near_plane, far_plane, fov, horizontal_aperture, vertical_aperture, focal_length, focus_distance;
    ExtractCameraData(shape, near_plane, far_plane, fov, horizontal_aperture, vertical_aperture, focal_length, focus_distance);

    float t = m_current_time * m_settings.animation_time_scale;
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

void MeshSyncClientMaya::extractLightAnimationData(ms::Animation& dst_, MObject node, MObject shape)
{
    extractTransformAnimationData(dst_, node, shape);

    auto& dst = (ms::LightAnimation&)dst_;
    {
        auto& last = dst.rotation.back();
        last.value = mu::flipY(last.value);
    }

    mu::float4 color;
    float intensity;
    float spot_angle;
    ExtractLightData(shape, color, intensity, spot_angle);

    float t = m_current_time * m_settings.animation_time_scale;
    dst.color.push_back({ t, color });
    dst.intensity.push_back({ t, intensity });
    if (shape.hasFn(MFn::kSpotLight)) {
        dst.spot_angle.push_back({ t, spot_angle });
    }
}
