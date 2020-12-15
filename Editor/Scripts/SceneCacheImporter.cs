using System.IO;
using UnityEngine;

#if UNITY_2020_2_OR_NEWER
using UnityEditor.AssetImporters;
#else
using UnityEditor.Experimental.AssetImporters;
#endif


namespace Unity.MeshSync.Editor {

[ScriptedImporter(1, "sc")]
internal class SceneCacheImporter : ScriptedImporter
{
    public override void OnImportAsset(AssetImportContext ctx) {
        //Ignore assets outside Assets folder (for example: Packages, etc)
        if (!ctx.assetPath.StartsWith("Assets/"))
            return;
        
        GameObject go = new GameObject();
        SceneCachePlayer player = go.AddComponent<SceneCachePlayer>();
        SceneCachePlayerEditorUtility.ChangeSceneCacheFile(player, ctx.assetPath);

        string objectName = Path.GetFileNameWithoutExtension(ctx.assetPath);
        go.name = objectName;
        
        ctx.AddObjectToAsset(objectName, go);
        ctx.SetMainObject(go);
    }
}

} //end namespace
