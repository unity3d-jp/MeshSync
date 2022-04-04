using System;
using UnityEditor;
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
                // Check if server is actually responding:
                if ((DateTime.Now - startTime).TotalSeconds > 10)
                {
                    EditorUtility.DisplayDialog("Blender is not responding", "Please ensure 'Auto-Sync' is enabled in the MeshSync plugin in blender!", "Ok");
                    startTime = DateTime.Now;
                    return true;
                }

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
