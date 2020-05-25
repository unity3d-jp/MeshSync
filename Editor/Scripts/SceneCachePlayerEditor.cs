using Unity.MeshSync;
using UnityEditor;
using UnityEngine;

namespace UnityEditor.MeshSync
{
    [CustomEditor(typeof(SceneCachePlayer))]
    internal class SceneCachePlayerEditor : MeshSyncPlayerEditor
    {
        [MenuItem("GameObject/MeshSync/Create Cache Player", false, 10)]
        public static void CreateSceneCachePlayerMenu(MenuCommand menuCommand)
        {
            var path = EditorUtility.OpenFilePanelWithFilters("Select Cache File", "",
                new string[]{ "All supported files", "sc", "All files", "*" });
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
            GameObject go = new GameObject();
            go.name = System.IO.Path.GetFileNameWithoutExtension(path);

            SceneCachePlayer player = go.AddComponent<SceneCachePlayer>();
            player.rootObject = go.GetComponent<Transform>();
            player.assetDir = new DataPath(DataPath.Root.DataPath, string.Format("SceneCache/{0}", go.name));
            player.markMeshesDynamic = true;
            player.dontSaveAssetsInScene = true;

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
            GameObject go = CreateSceneCachePlayer(path);
            if (go == null)
                return null;

            // export materials & animation and generate prefab
            SceneCachePlayer player = go.GetComponent<SceneCachePlayer>();
            player.UpdatePlayer();
            player.ExportMaterials(false, true);
            player.ResetTimeAnimation();
            player.handleAssets = false;
            SceneData scene = player.GetLastScene();
            if (!scene.submeshesHaveUniqueMaterial) {
                MeshSyncPlayerConfig config = player.GetConfig();
                config.SyncMaterialList = false;
            }

            string prefabPath = string.Format("Assets/SceneCache/{0}.prefab", go.name);
            PrefabUtility.SaveAsPrefabAssetAndConnect(go, prefabPath, InteractionMode.AutomatedAction);
            return go;
        }


        public override void OnInspectorGUI()
        {
            var so = serializedObject;
            SceneCachePlayer t = target as SceneCachePlayer;

            EditorGUILayout.Space();
            DrawCacheSettings(t, so);
            DrawPlayerSettings(t, so);
            MeshSyncPlayerConfig config = t.GetConfig();
            if (config.Profiling)
            {
                EditorGUILayout.TextArea(t.dbgProfileReport, GUILayout.Height(120));
                EditorGUILayout.Space();
            }

            DrawMaterialList(t);
            DrawTextureList(t);
            DrawAnimationTweak(t);
            DrawExportAssets(t);
            DrawPluginVersion();

            so.ApplyModifiedProperties();
        }

        void DrawCacheSettings(SceneCachePlayer t, SerializedObject so)
        {
            var styleFold = EditorStyles.foldout;
            styleFold.fontStyle = FontStyle.Bold;

            t.foldCacheSettings = EditorGUILayout.Foldout(t.foldCacheSettings, "Player", true, styleFold);
            if (t.foldCacheSettings)
            {
                // cache file path
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
                        Undo.RecordObject(t, "SceneCachePlayer");
                        t.OpenCache(dstPath);
                        Repaint();
                    }
                    if (GUILayout.Button("Move to StreamingAssets", GUILayout.Width(160.0f)))
                    {
                        t.CloseCache();
                        var srcPath = dataPath.fullPath;
                        var dstPath = Misc.MoveFileToStreamingAssets(dataPath.fullPath);
                        Undo.RecordObject(t, "SceneCachePlayer");
                        t.OpenCache(dstPath);
                        Repaint();
                    }
                    GUILayout.EndHorizontal();
                }
                EditorGUILayout.Space();


                // time / frame
                System.Action resetTimeAnimation = () =>
                {
                    so.ApplyModifiedProperties();
                    t.ResetTimeAnimation();
                };

                EditorGUI.BeginChangeCheck();
                EditorGUILayout.PropertyField(so.FindProperty("m_timeUnit"));
                if (EditorGUI.EndChangeCheck())
                    resetTimeAnimation();

                if (t.timeUnit == SceneCachePlayer.TimeUnit.Seconds)
                {
                    EditorGUILayout.PropertyField(so.FindProperty("m_time"));
                    EditorGUILayout.PropertyField(so.FindProperty("m_interpolation"));
                }
                else if (t.timeUnit == SceneCachePlayer.TimeUnit.Frames)
                {
                    EditorGUI.BeginChangeCheck();
                    EditorGUILayout.PropertyField(so.FindProperty("m_baseFrame"));
                    if (EditorGUI.EndChangeCheck())
                        resetTimeAnimation();

                    EditorGUILayout.PropertyField(so.FindProperty("m_frame"));
                }

                // preload
                {
                    var preloadLength = so.FindProperty("m_preloadLength");
                    preloadLength.intValue = EditorGUILayout.IntSlider("Preload Length", preloadLength.intValue, 0, t.frameCount);
                }

                EditorGUILayout.Space();
            }
        }
    }
} //end namespace
