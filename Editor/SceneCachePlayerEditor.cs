using UnityEditor;
using UnityEngine;
using UTJ.MeshSync;

namespace UTJ.MeshSyncEditor
{
    [CustomEditor(typeof(SceneCachePlayer))]
    public class SceneCachePlayerEditor : Editor
    {
#if UNITY_EDITOR
        [MenuItem("GameObject/MeshSync/Create Cache Player", false, 10)]
        public static void CreateSceneCachePlayer(MenuCommand menuCommand)
        {
            var path = EditorUtility.OpenFilePanel("Select Cache File", "", "");
            if (path.Length > 0)
            {
                var go = new GameObject();
                go.name = System.IO.Path.GetFileNameWithoutExtension(path);

                var server = go.AddComponent<MeshSyncServer>();
                server.rootObject = go.GetComponent<Transform>();
                server.progressiveDisplay = false;

                var player = go.AddComponent<SceneCachePlayer>();
                if (!player.OpenCache(path))
                {
                    Debug.LogError("Failed to open " + path + ".\nPossible reasons: the file is not scene cache, or file format version does not match.");
                    DestroyImmediate(go);
                }
                else
                {
                    player.AddAnimator("Assets");
                    Undo.RegisterCreatedObjectUndo(go, "SceneCachePlayer");
                }
            }
        }
#endif

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
                    var dstPath = Misc.CopyFileToStreamingAssets(dataPath.fullPath);
                    t.OpenCache(dstPath);
                    Repaint();
                }
            }

            EditorGUILayout.Space();
            EditorGUILayout.LabelField("Plugin Version: " + MeshSyncServer.pluginVersion);
        }
    }
}
