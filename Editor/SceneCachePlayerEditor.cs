using UnityEditor;
using UnityEngine;
using UTJ.MeshSync;

namespace UTJ.MeshSyncEditor
{
    [CustomEditor(typeof(SceneCachePlayer))]
    public class SceneCachePlayerEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            DrawDefaultInspector();

            var t = target as SceneCachePlayer;

            EditorGUILayout.Space();

            var dataPath = t.cacheFilePath;
            if (dataPath.root != DataPath.Root.StreamingAssets)
            {
                if (GUILayout.Button("Copy to StreamingAssets"))
                {
                    var srcPath = dataPath.fullPath;
                    var dstPath = CopyFileToStreamingAssets(dataPath.fullPath);
                    t.OpenCache(dstPath);
                    Repaint();
                }
            }

            EditorGUILayout.Space();
            EditorGUILayout.LabelField("Plugin Version: " + MeshSyncServer.pluginVersion);
        }

        string CopyFileToStreamingAssets(string srcPath)
        {
            var streaminAssetsPath = Application.streamingAssetsPath;
            if (srcPath.Contains(streaminAssetsPath))
                return srcPath;

            var filename = System.IO.Path.GetFileNameWithoutExtension(srcPath);
            var ext = System.IO.Path.GetExtension(srcPath);
            for (int n = 1; ; ++n)
            {
                var ns = n == 1 ? "" : n.ToString();
                var dstPath = string.Format("{0}/{1}{2}{3}", streaminAssetsPath, filename, ns, ext);
                if (System.IO.File.Exists(dstPath))
                {
                    var srcInfo = new System.IO.FileInfo(srcPath);
                    var dstInfo = new System.IO.FileInfo(dstPath);
                    bool identical =
                        srcInfo.Length == dstInfo.Length &&
                        srcInfo.LastWriteTime == dstInfo.LastWriteTime;
                    if (identical)
                    {
                        Debug.Log(string.Format("SceneCachePlayer: use existing file {0} -> {1}", srcPath, dstPath));
                        return dstPath;
                    }
                    else
                        continue;
                }
                else
                {
                    System.IO.File.Copy(srcPath, dstPath);
                    Debug.Log(string.Format("SceneCachePlayer: copy {0} -> {1}", srcPath, dstPath));
                    return dstPath;
                }
            }
        }
    }
}
