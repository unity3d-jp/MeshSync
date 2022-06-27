using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using System.IO;

namespace Unity.MeshSync.Editor
{
    public static class MeshSyncServerInspectorUtils
    {
        public static void OpenDCCAsset(MeshSyncServer server)
        {
            var previousRunMode = server.m_DCCInterop != null ? server.m_DCCInterop.runMode : RunMode.GUI;

            var asset = server.m_DCCAsset;

            server.m_DCCInterop?.Dispose();
            server.m_DCCInterop = GetLauncherForAsset(asset);

            if (server.m_DCCInterop != null)
            {
                server.m_DCCInterop.runMode = previousRunMode;
                server.m_DCCInterop.OpenDCCTool(asset);
            }
            else
            {
                var assetPath = AssetDatabase.GetAssetPath(asset);
                var extension = Path.GetExtension(assetPath);
                Debug.LogError($"No DCC handler for {extension} files is implemented.");
            }
        }

        internal static IDCCLauncher GetLauncherForAsset(UnityEngine.Object asset)
        {
            if (asset == null)
            {
                return null;
            }

            var assetPath = AssetDatabase.GetAssetPath(asset);

            if (Path.GetExtension(assetPath) == ".blend")
            {
                return new BlenderLauncher();
            }

            // TODO: Implement and return launchers for other DCC file types here:

            return null;
        }
    }
}
