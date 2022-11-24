using System.Collections.Generic;
using UnityEngine;
#if AT_USE_SPLINES
using UnityEngine.Splines;
using Unity.Mathematics;
#endif

#if AT_USE_PROBUILDER && UNITY_EDITOR
using UnityEngine.ProBuilder;
#endif

namespace Unity.MeshSync
{
    partial class MeshSyncServer
    {
        internal enum PropertiesState
        {
            None,
            Sending,
            Received
        }

        bool needsClientSync;

        Dictionary<EntityRecord, Matrix4x4> entityTransforms = new Dictionary<EntityRecord, Matrix4x4>();

#if UNITY_EDITOR
        internal override InstanceHandlingType InstanceHandling
        {
            get => base.InstanceHandling; 
            set
            {
                if (InstanceHandling != value)
                {
                    base.InstanceHandling = value;

                    ClearInstancePrefabs();
                }
            }
        }
#endif

#if AT_USE_SPLINES
        List<string> changedSplines = new List<string>();
#endif

#if AT_USE_PROBUILDER && UNITY_EDITOR

        HashSet<ProBuilderMesh> changedMeshes = new HashSet<ProBuilderMesh>();
        
        internal override bool UseProBuilder
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

        internal int InstanceCount
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

        internal PropertiesState CurrentPropertiesState { get; private set; }
        internal void ResetPropertiesState()
        {
            CurrentPropertiesState = PropertiesState.None;
        }

        /// <summary>
        /// True when the connection to the DCC tool to send data is active.
        /// </summary>
        public bool IsDCCLiveEditReady()
        {
            return m_server.IsDCCLiveEditReady();
        }

        void SendUpdatedProperties()
        {
            if (!m_server.IsDCCLiveEditReady())
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

                        MeshSyncLogger.VerboseLog($"Sending changes, property '{prop.name}' was dirty.");
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

#if AT_USE_SPLINES
                        if (changedSplines.Count > 0 && entity.dataType == EntityType.Curve)
                        {
                            //m_server.SendCurve(entity, kvp.Key);
                            SendCurve(kvp.Key, entity);
                            sendChanges = true;
                        }
#endif
#if AT_USE_PROBUILDER && UNITY_EDITOR
                        if (entity.dataType == EntityType.Mesh && entity.proBuilderMeshFilter != null)
                        {
                            if (changedMeshes.Contains(entity.proBuilderMeshFilter))
                            {
                                SendMesh(kvp.Key, entity);
                                sendChanges = true;
                            }
                        }
#endif
                    }

#if AT_USE_SPLINES
                    changedSplines.Clear();
#endif

#if AT_USE_PROBUILDER && UNITY_EDITOR
                    changedMeshes.Clear();
#endif
                }

                if (needsClientSync)
                {
                    MeshSyncLogger.VerboseLog("Sending changes, needed client sync.");

                    needsClientSync = false;
                    sendChanges = true;

                    m_server.RequestClientSync();
                }

                if (sendChanges)
                {
                    CurrentPropertiesState = PropertiesState.Sending;
                    m_server.MarkServerInitiatedResponseReady();
                }
            }
        }

        void SendTransform(string path, EntityRecord entity)
        {
            // TODO: Implement this. https://jira.unity3d.com/browse/BLENDER-478
        }


#if AT_USE_PROBUILDER && UNITY_EDITOR
        internal void MeshChanged(ProBuilderMesh mesh) {
            lock (PropertyInfoDataWrapper.PropertyUpdateLock) {
                changedMeshes.Add(mesh);
            }
        }

        static GetFlags sendMeshSettings = new GetFlags(
            GetFlags.GetFlagsSetting.Transform, 
            GetFlags.GetFlagsSetting.Points,
            GetFlags.GetFlagsSetting.Indices, 
            GetFlags.GetFlagsSetting.MaterialIDS);

        void SendMesh(string path, EntityRecord entity)
        {
            Debug.Assert(entity.dataType == EntityType.Mesh);

            var objRenderer = entity.meshRenderer;

            MeshData dst = MeshData.Create();

            var mesh = entity.meshFilter.sharedMesh;
            var mr = entity.meshRenderer;

            CaptureMesh(ref dst, mesh, null,
                sendMeshSettings,
                mr.sharedMaterials);

            TransformData dstTrans = dst.transform;
            Transform rendererTransform = objRenderer.transform;
            dstTrans.hostID = GetObjectlID(objRenderer.gameObject);
            dstTrans.position = rendererTransform.localPosition;
            dstTrans.rotation = rendererTransform.localRotation;
            dstTrans.scale = rendererTransform.localScale;
            dst.local2world = rendererTransform.localToWorldMatrix;
            dst.world2local = rendererTransform.worldToLocalMatrix;

            dstTrans.path = path;
            
            m_server.SendMesh(dst);
        }
#endif

#if AT_USE_SPLINES

        void SendCurve(string path, EntityRecord entity)
        {
            Debug.Assert(entity.dataType == EntityType.Curve);
            
            for (int splineIdx = 0; splineIdx < entity.splineContainer.Splines.Count; splineIdx++)
            {
                var spline = entity.splineContainer.Splines[splineIdx];

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

        internal override void AfterUpdateScene() {
            base.AfterUpdateScene();

            CurrentPropertiesState = PropertiesState.Received;
        }

#if UNITY_EDITOR
        internal override void ClearInstancePrefabs()
        {
            base.ClearInstancePrefabs();

            needsClientSync = true;
        }
#endif
    }
}
