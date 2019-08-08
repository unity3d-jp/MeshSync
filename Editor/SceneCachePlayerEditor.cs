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

                var player = go.AddComponent<SceneCachePlayer>();
                player.rootObject = go.GetComponent<Transform>();
                player.progressiveDisplay = false;
                player.dontSaveAssetsInScene = true;
                if (!player.OpenCache(path))
                {
                    Debug.LogError("Failed to open " + path + ".\nPossible reasons: the file is not scene cache, or file format version does not match.");
                    DestroyImmediate(go);
                    return;
                }
                else
                {
                    player.AddAnimator("Assets");
                    Undo.RegisterCreatedObjectUndo(go, "SceneCachePlayer");
                }

                //player.UpdatePlayer();
                //player.ExportMaterials();
                //var prefab = PrefabUtility.SaveAsPrefabAsset(go, "Assets/" + go.name + ".prefab");
                //var index = go.transform.GetSiblingIndex();
                ////Object.DestroyImmediate(go);
                //var inst = PrefabUtility.InstantiatePrefab(prefab) as GameObject;
                //inst.transform.SetSiblingIndex(index);
            }
        }
#endif

        public override void OnInspectorGUI()
        {
            var so = serializedObject;
            var t = target as SceneCachePlayer;

            EditorGUILayout.LabelField("Player", EditorStyles.boldLabel);
            EditorGUILayout.PropertyField(so.FindProperty("m_cacheFilePath"));
            var dataPath = t.cacheFilePath;
            if (dataPath.root != DataPath.Root.StreamingAssets)
            {
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("Copy to StreamingAssets", GUILayout.Width(160.0f)))
                {
                    var srcPath = dataPath.fullPath;
                    var dstPath = Misc.CopyFileToStreamingAssets(dataPath.fullPath);
                    t.OpenCache(dstPath);
                    Repaint();
                }
                if (GUILayout.Button("Move to StreamingAssets", GUILayout.Width(160.0f)))
                {
                    t.CloseCache();
                    var srcPath = dataPath.fullPath;
                    var dstPath = Misc.MoveFileToStreamingAssets(dataPath.fullPath);
                    t.OpenCache(dstPath);
                    Repaint();
                }
                GUILayout.EndHorizontal();
            }
            EditorGUILayout.Space();

            EditorGUILayout.PropertyField(so.FindProperty("m_time"));
            EditorGUILayout.PropertyField(so.FindProperty("m_interpolation"));
            EditorGUILayout.Space();

            MeshSyncServerEditor.DrawPlayerSettings(t, so);
            MeshSyncServerEditor.DrawMaterialList(t);
            MeshSyncServerEditor.DrawTextureList(t);

            EditorGUILayout.LabelField("Plugin Version: " + MeshSyncPlayer.pluginVersion);
            so.ApplyModifiedProperties();
        }
    }
}
