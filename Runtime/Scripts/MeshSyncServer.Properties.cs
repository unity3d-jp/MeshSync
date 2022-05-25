using System.Collections.Generic;
using UnityEngine;

#if MESHSYNC_SPLINE_SUPPORT
using UnityEngine.Splines;
using Unity.Mathematics;
#endif

namespace Unity.MeshSync
{
    partial class MeshSyncServer
    {
        public enum PropertiesState
        {
            None,
            Sending,
            Received
        };

        bool needsClientSync;

        Dictionary<EntityRecord, Matrix4x4> entityTransforms = new Dictionary<EntityRecord, Matrix4x4>();
        public override InstanceHandlingType InstanceHandling
        {
            get => base.InstanceHandling; set
            {
                if (InstanceHandling != value)
                {
                    base.InstanceHandling = value;

                    ClearInstancePrefabs();
                }
            }
        }

#if MESHSYNC_SPLINE_SUPPORT
        List<string> changedSplines = new List<string>();
#endif

#if MESHSYNC_PROBUILDER_SUPPORT
        HashSet<UnityEngine.ProBuilder.ProBuilderMesh> changedMeshes = new HashSet<UnityEngine.ProBuilder.ProBuilderMesh>();

        public override bool UseProBuilder
        {
            get => base.UseProBuilder;
            set
            {
                if (UseProBuilder != value)
                {
                    base.UseProBuilder = value;

                    ClearInstancePrefabs();
                }
            }
        }
#endif

        public int InstanceCount
        {
            get
            {
                int count = 0;

                foreach (var inst in m_clientInstances)
                {
                    count += inst.Value.instanceObjects.Count;

                    if (inst.Value.renderer != null)
                    {
                        count += inst.Value.renderer.TransformCount;
                    }
                }
                return count;
            }
        }

        public PropertiesState CurrentPropertiesState { get; private set; }
        public void ResetPropertiesState()
        {
            CurrentPropertiesState = PropertiesState.None;
        }

        public override void ClearInstancePrefabs()
        {
            base.ClearInstancePrefabs();

            needsClientSync = true;
        }

        public bool IsConnectionToBlenderActive()
        {
            return m_server.CanServerReceiveProperties();
        }

        void SendUpdatedProperties()
        {
            if (!m_server.CanServerReceiveProperties())
            {
                return;
            }

            lock (PropertyInfoDataWrapper.PropertyUpdateLock)
            {
                bool sendChanges = false;

                // Send updated properties:
                foreach (var prop in propertyInfos)
                {
                    if (prop.IsDirty)
                    {
                        m_server.SendProperty(prop);
                        prop.IsDirty = false;
                        sendChanges = true;
                    }
                }

                // Send updated entities:
                {
                    foreach (var kvp in GetClientObjects())
                    {
                        var entity = kvp.Value;

                        if (entity.trans == null)
                        {
                            continue;
                        }

                        if (!entityTransforms.TryGetValue(entity, out var matrix))
                        {
                            entityTransforms.Add(entity, entity.trans.localToWorldMatrix);
                        }
                        else
                        {
                            // Check if object moved:
                            if (entity.trans.localToWorldMatrix != matrix)
                            {
                                SendTransform(kvp.Key, entity);

                                entityTransforms[entity] = entity.trans.localToWorldMatrix;
                            }
                        }

#if MESHSYNC_SPLINE_SUPPORT
                        if (changedSplines.Count > 0 && entity.dataType == EntityType.Curve)
                        {
                            //m_server.SendCurve(entity, kvp.Key);
                            SendCurve(kvp.Key, entity);
                            sendChanges = true;
                        }
#endif
#if MESHSYNC_PROBUILDER_SUPPORT
                        if (entity.dataType == EntityType.Mesh)
                        {
                            if (changedMeshes.Contains(entity.proBuilderMeshFilter))
                            {
                                SendMesh(kvp.Key, entity);
                                sendChanges = true;
                            }
                        }
#endif
                    }

#if MESHSYNC_SPLINE_SUPPORT
                    changedSplines.Clear();
#endif

#if MESHSYNC_PROBUILDER_SUPPORT
                    changedMeshes.Clear();
#endif
                }

                if (needsClientSync)
                {
                    needsClientSync = false;
                    sendChanges = true;

                    m_server.RequestClientSync();
                }

                if (sendChanges)
                {
                    CurrentPropertiesState = PropertiesState.Sending;
                    onSceneUpdateEnd -= SceneUpdated;
                    onSceneUpdateEnd += SceneUpdated;
                    m_server.SendChangedProperties();
                }
            }
        }

        void SendTransform(string path, EntityRecord entity)
        {

        }


#if MESHSYNC_PROBUILDER_SUPPORT
        public void MeshChanged(UnityEngine.ProBuilder.ProBuilderMesh mesh)
        {
            changedMeshes.Add(mesh);
        }

        void SendMesh(string path, EntityRecord entity)
        {
            Debug.Assert(entity.dataType == EntityType.Mesh);



            var objRenderer = entity.meshRenderer;

            //Mesh origMesh = null;
            MeshData dst = MeshData.Create();

            var mesh = entity.meshFilter.sharedMesh;
            var mr = entity.meshRenderer;

            CaptureMesh(ref dst, mesh, null,
                new GetFlags(GetFlags.AllGetFlagsSetting()),
                mr.sharedMaterials);

            // TODO: Maybe support skinned mesh renderers too:
            //if (objRenderer.GetType() == typeof(MeshRenderer)){
            //    ret = CaptureMeshRenderer(ref dst, objRenderer as MeshRenderer, mes, ref origMesh);
            //}
            //else if (objRenderer.GetType() == typeof(SkinnedMeshRenderer)){
            //    ret = CaptureSkinnedMeshRenderer(ref dst, objRenderer as SkinnedMeshRenderer, mes, ref origMesh);
            //}

            TransformData dstTrans = dst.transform;
            Transform rendererTransform = objRenderer.transform;
            dstTrans.hostID = GetObjectlID(objRenderer.gameObject);
            dstTrans.position = rendererTransform.localPosition;
            dstTrans.rotation = rendererTransform.localRotation;
            dstTrans.scale = rendererTransform.localScale;
            dst.local2world = rendererTransform.localToWorldMatrix;
            dst.world2local = rendererTransform.worldToLocalMatrix;

            //EntityRecord rec;
            //if (!m_hostObjects.TryGetValue(dstTrans.hostID, out rec))
            //{
            //    rec = new EntityRecord();
            //    m_hostObjects.Add(dstTrans.hostID, rec);
            //}
            //entity.go = objRenderer.gameObject;
            //entity.origMesh = origMesh;

            dstTrans.path = path;// BuildPath(rendererTransform);
            //m_server.ServeMesh(dst);

            m_server.SendMesh(dst);
        }
#endif

#if MESHSYNC_SPLINE_SUPPORT

        void SendCurve(string path, EntityRecord entity)
        {
            Debug.Assert(entity.dataType == EntityType.Curve);

            for (int splineIdx = 0; splineIdx < entity.splineContainer.Branches.Count; splineIdx++)
            {
                var spline = entity.splineContainer.Branches[splineIdx];

                var pointCount = spline.Count;
                if (pointCount == 0)
                {
                    continue;
                }

                var cos = new float3[pointCount];
                var handlesLeft = new float3[pointCount];
                var handlesRight = new float3[pointCount];

                for (int pointIdx = 0; pointIdx < pointCount; pointIdx++)
                {
                    var knot = spline[pointIdx];

                    var co = knot.Position;

                    cos[pointIdx] = co;

                    handlesLeft[pointIdx] = math.rotate(knot.Rotation, knot.TangentIn) + co;
                    handlesRight[pointIdx] = math.rotate(knot.Rotation, knot.TangentOut) + co;
                }

                m_server.SendCurve(path, splineIdx, pointCount, spline.Closed, cos, handlesLeft, handlesRight);
            }
        }

        protected override void SplineChanged(Spline spline, int arg2, SplineModification arg3)
        {
            // TODO: Get the path here:
            changedSplines.Add(spline.ToString());
        }
#endif

        void OnRecvPropertyRequest()
        {
        }

        void SceneUpdated()
        {
            onSceneUpdateEnd -= SceneUpdated;

            CurrentPropertiesState = PropertiesState.Received;
        }
    }
}
