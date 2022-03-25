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

        public override bool keepWaiting => server.CurrentPropertiesState != MeshSyncServer.PropertiesState.Received;
    }

}
