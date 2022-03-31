using UnityEngine;

namespace Unity.MeshSync.VariantExport
{
    class WaitForMeshSync : CustomYieldInstruction
    {
        MeshSyncServer server;

        public WaitForMeshSync(MeshSyncServer server)
        {
            this.server = server;
        }

        public override bool keepWaiting
        {
            get
            {
                if (server.CurrentPropertiesState != MeshSyncServer.PropertiesState.Received)
                {
                    return true;
                }

                server.ResetPropertiesState();

                return false;
            }
        }
    }
}
