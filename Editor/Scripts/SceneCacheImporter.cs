using Unity.MeshSync;
using UnityEngine;
using UnityEditor;
using UnityEditor.Experimental.AssetImporters;


namespace Unity.MeshSync.Editor
{
    [ScriptedImporter(1, "sc")]
    internal class SceneCacheImporter : ScriptedImporter
    {
        public override void OnImportAsset(AssetImportContext ctx) {
            //Ignore assets outside Assets folder
            string path = ctx.assetPath;
            if (!path.StartsWith(Application.dataPath))
                return;
            
            GameObject go = SceneCachePlayerEditor.CreateSceneCachePlayer(path);
            if (go == null)
                return;

            // export materials & animation and generate prefab
            SceneCachePlayer player = go.GetComponent<SceneCachePlayer>();
            player.UpdatePlayer();
            player.ExportMaterials(false, true);
            player.ResetTimeAnimation();
            player.handleAssets = false;

            ctx.AddObjectToAsset(go.name, go);
            ctx.SetMainObject(go);
        }
    }
}
