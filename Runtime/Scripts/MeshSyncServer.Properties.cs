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

        public PropertiesState CurrentPropertiesState { get; private set; }
        public void ResetPropertiesState()
        {
            CurrentPropertiesState = PropertiesState.None;
        }

        void SendUpdatedProperties()
        {
            lock (PropertyInfoDataWrapper.PropertyUpdateLock)
            {
                bool sendProps = false;

                foreach (var prop in propertyInfos)
                {
                    if (prop.IsDirty)
                    {
                        m_server.SendProperty(prop);
                        prop.IsDirty = false;
                        sendProps = true;
                    }
                }

                if (sendProps)
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
