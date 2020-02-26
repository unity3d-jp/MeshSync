using Unity.MeshSync;
using UnityEngine;
using UnityEditor;
using UnityEditor.Experimental.AssetImporters;


namespace Unity.MeshSyncEditor
{
    [ScriptedImporter(1, "sc")]
    public class SceneCacheImporter : ScriptedImporter
    {
        public override void OnImportAsset(AssetImportContext ctx)
        {
            var path = ctx.assetPath;
            var go = SceneCachePlayerEditor.CreateSceneCachePlayer(path);
            if (go == null)
                return;

            // export materials & animation and generate prefab
            var player = go.GetComponent<SceneCachePlayer>();
            player.UpdatePlayer();
            player.ExportMaterials(false, true);
            player.ResetTimeAnimation();
            player.handleAssets = false;

            ctx.AddObjectToAsset(go.name, go);
            ctx.SetMainObject(go);
        }
    }
}
