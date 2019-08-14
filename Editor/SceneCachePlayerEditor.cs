using UnityEditor;
using UnityEngine;
using UTJ.MeshSync;

namespace UTJ.MeshSyncEditor
{
    [CustomEditor(typeof(SceneCachePlayer))]
    public class SceneCachePlayerEditor : MeshSyncPlayerEditor
    {
        [MenuItem("GameObject/MeshSync/Create Cache Player", false, 10)]
        public static void CreateSceneCachePlayerMenu(MenuCommand menuCommand)
        {
            var path = EditorUtility.OpenFilePanel("Select Cache File", "", "");
            if (path.Length > 0)
            {
                var go = CreateSceneCachePlayerPrefab(path);
                if (go != null)
                    Undo.RegisterCreatedObjectUndo(go, "SceneCachePlayer");
            }
        }

        public static GameObject CreateSceneCachePlayer(string path)
        {
            // create temporary GO to make prefab
            var go = new GameObject();
            go.name = System.IO.Path.GetFileNameWithoutExtension(path);

            var player = go.AddComponent<SceneCachePlayer>();
            player.rootObject = go.GetComponent<Transform>();
            player.assetDir = new DataPath(DataPath.Root.DataPath, string.Format("SceneCache/{0}", go.name));
            player.updateMeshColliders = false;
            player.progressiveDisplay = false;
            player.dontSaveAssetsInScene = true;
            player.findMaterialFromAssets = false;
            if (!player.OpenCache(path))
            {
                Debug.LogError("Failed to open " + path + ". Possible reasons: file format version does not match, or the file is not scene cache.");
                DestroyImmediate(go);
                return null;
            }
            return go;
        }

        public static GameObject CreateSceneCachePlayerPrefab(string path)
        {
            var go = CreateSceneCachePlayer(path);
            if (go == null)
                return null;

            // export materials & animation and generate prefab
            var player = go.GetComponent<SceneCachePlayer>();
            player.UpdatePlayer();
            player.ExportMaterials(false, true);
            player.AddAnimator();
            player.handleAssets = false;

            var prefabPath = string.Format("Assets/SceneCache/{0}.prefab", go.name);
            var prefab = PrefabUtility.SaveAsPrefabAsset(go, prefabPath);

            // delete temporary GO and instantiate prefab
            var index = go.transform.GetSiblingIndex();
            Object.DestroyImmediate(go);
            var inst = PrefabUtility.InstantiatePrefab(prefab) as GameObject;
            inst.transform.SetSiblingIndex(index);
            return inst;
        }


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

            MeshSyncPlayerEditor.DrawPlayerSettings(t, so);

            if (t.profiling)
            {
                EditorGUILayout.TextArea(t.dbgProfileReport, GUILayout.Height(120));
                EditorGUILayout.Space();
            }

            MeshSyncPlayerEditor.DrawMaterialList(t);
            MeshSyncPlayerEditor.DrawTextureList(t);
            this.DrawAnimationSettings(t);

            EditorGUILayout.LabelField("Plugin Version: " + MeshSyncPlayer.pluginVersion);
            so.ApplyModifiedProperties();
        }
    }
}
