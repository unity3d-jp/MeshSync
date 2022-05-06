using Unity.Mathematics;

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
        bool splineChanged;

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
                if (splineChanged)
                {
                    splineChanged = false;

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

        protected override void SplineChanged()
        {
            splineChanged = true;
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
