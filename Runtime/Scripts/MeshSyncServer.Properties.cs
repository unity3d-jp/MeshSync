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
                foreach (var kvp in GetClientObjects())
                {
                    var entity = kvp.Value;
                    if (entity.dataType != EntityType.Curve)
                    {
                        continue;
                    }

                    if (entity.curve.IsDirty)
                    {
                        m_server.SendCurve(entity);
                        entity.curve.IsDirty = false;
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
