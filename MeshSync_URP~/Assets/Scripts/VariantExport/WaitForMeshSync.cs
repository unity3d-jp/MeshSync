using System;
using UnityEngine;

namespace Unity.MeshSync.VariantExport
{
    class WaitForMeshSync : CustomYieldInstruction
    {
        MeshSyncServer server;
        DateTime startTime;

        public WaitForMeshSync(MeshSyncServer server)
        {
            this.server = server;
            startTime = DateTime.Now;
        }

        public override bool keepWaiting
        {
            get
            {
                // If server took too long to respond, we may need to reconnect:
                //if ((DateTime.Now - startTime).TotalSeconds > 10)
                //{
                //    server.StartServer();
                //    startTime = DateTime.Now;
                //}

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
