using System.Collections.Generic;
using Unity.Mathematics;
using UnityEngine.Splines;

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
        List<string> changedSplines = new List<string>();

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

                // Send updated curves:
                if (changedSplines.Count > 0)
                {
                    var floatList = new PinnedList<float3>();

                    foreach (var kvp in GetClientObjects())
                    {
                        var entity = kvp.Value;
                        if (entity.dataType != EntityType.Curve)
                        {
                            continue;
                        }

                        m_server.SendCurve(entity, kvp.Key);
                        sendChanges = true;
                    }

                    changedSplines.Clear();
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

        protected override void SplineChanged(Spline spline, int arg2, SplineModification arg3)
        {
            // TODO: Get the path here:
            changedSplines.Add(spline.ToString());
        }

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
