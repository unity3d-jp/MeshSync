#define _MApiVersion
#include "pch.h"
#include "MayaUtils.h"
#include "MeshSyncClientMaya.h"
#include "Commands.h"


void MeshSyncClientMaya::extractSceneData()
{
    // materials
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
    if (m_settings.sync_animations && MAnimUtil::isAnimated(src)) {
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

            // this copy is inevitable..
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
            int sps = m_settings.sample_animation ? m_settings.animation_sps : 0;
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
    if (m_settings.sync_animations && MAnimUtil::isAnimated(shape)) {
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
            int sps = m_settings.sample_animation ? m_settings.animation_sps : 0;
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
                if (!fcv.object().isNull() && acv.object().isNull()) {
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
    if (m_settings.sync_animations && MAnimUtil::isAnimated(shape)) {
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
            int sps = m_settings.sample_animation ? m_settings.animation_sps : 0;
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
        // return empty mesh
        return true;
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

        // apply tweaks
        if (m_settings.apply_tweak) {
            apply_tweak(fn_blendshape.object(), skin_index);
            apply_uv_tweak(fn_blendshape.object(), skin_index);
        }
    }

    dst.setupFlags();
    return true;
}
