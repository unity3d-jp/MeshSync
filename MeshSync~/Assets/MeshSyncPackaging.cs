#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;


public class MeshSyncPackaging
{
    [MenuItem("Assets/Make MeshSync.unitypackage")]
    public static void MakePackage()
    {
        string[] files = new string[]
        {
            "Assets/UTJ/MeshSync",
            "Assets/StreamingAssets/MeshSyncServerRoot",
        };
        AssetDatabase.ExportPackage(files, "MeshSync.unitypackage", ExportPackageOptions.Recurse);
    }

}
#endif // UNITY_EDITOR
