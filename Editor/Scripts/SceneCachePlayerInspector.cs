using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor  {

[CustomEditor(typeof(SceneCachePlayer))]
internal class SceneCachePlayerInspector : MeshSyncPlayerInspector {
    

//----------------------------------------------------------------------------------------------------------------------

    public override void OnEnable() {
        base.OnEnable();
        m_sceneCachePlayer = target as SceneCachePlayer;
        
    }


//----------------------------------------------------------------------------------------------------------------------
    
    
    
    public override void OnInspectorGUI() {
        SerializedObject so = serializedObject;

        EditorGUILayout.Space();
        DrawCacheSettings(so);
        DrawPlayerSettings(m_sceneCachePlayer, so);
        MeshSyncPlayerConfig config = m_sceneCachePlayer.GetConfig();
        if (config.Profiling) {
            EditorGUILayout.TextArea(m_sceneCachePlayer.dbgProfileReport, GUILayout.Height(120));
            EditorGUILayout.Space();
        }

        DrawMaterialList(m_sceneCachePlayer);
        DrawTextureList(m_sceneCachePlayer);
        DrawAnimationTweak(m_sceneCachePlayer);
        DrawExportAssets(m_sceneCachePlayer);
        DrawPluginVersion();

        so.ApplyModifiedProperties();
    }

//----------------------------------------------------------------------------------------------------------------------
    
    void DrawCacheSettings(SerializedObject so) {
        GUIStyle styleFold = EditorStyles.foldout;
        styleFold.fontStyle = FontStyle.Bold;

        m_sceneCachePlayer.foldCacheSettings = EditorGUILayout.Foldout(m_sceneCachePlayer.foldCacheSettings, "Player", true, styleFold);
        if (m_sceneCachePlayer.foldCacheSettings) {
            // cache file path
            string fullPath = m_sceneCachePlayer.GetFilePath();
            using (new EditorGUI.DisabledScope(true)) {               
                EditorGUILayout.TextField("Cache File Path", AssetUtility.NormalizeAssetPath(fullPath));
            }
            if (!string.IsNullOrEmpty(fullPath) && !fullPath.StartsWith(Application.streamingAssetsPath)) {
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                const float BUTTON_WIDTH = 50.0f;
                if (GUILayout.Button("Copy", GUILayout.Width(BUTTON_WIDTH))) {
                    string dstPath = Misc.CopyFileToStreamingAssets(fullPath);
                    OnFilePathChanged(m_sceneCachePlayer, dstPath);
                }
                GUILayout.Label("or");
                if (GUILayout.Button("Move", GUILayout.Width(BUTTON_WIDTH))) {
                    m_sceneCachePlayer.CloseCache();
                    string dstPath = Misc.MoveFileToStreamingAssets(fullPath);
                    OnFilePathChanged(m_sceneCachePlayer, dstPath);
                }
                GUILayout.Label("to StreamingAssets");
                GUILayout.EndHorizontal();
            }
            EditorGUILayout.Space();

            // time / frame
            System.Action resetTimeAnimation = () => {
                so.ApplyModifiedProperties();
                m_sceneCachePlayer.ResetTimeAnimation();
            };

            EditorGUI.BeginChangeCheck();
            EditorGUILayout.PropertyField(so.FindProperty("m_timeUnit"));
            if (EditorGUI.EndChangeCheck())
                resetTimeAnimation();

            if (m_sceneCachePlayer.timeUnit == SceneCachePlayer.TimeUnit.Seconds) {
                EditorGUILayout.PropertyField(so.FindProperty("m_time"));
                EditorGUILayout.PropertyField(so.FindProperty("m_interpolation"));
            } else if (m_sceneCachePlayer.timeUnit == SceneCachePlayer.TimeUnit.Frames) {
                EditorGUI.BeginChangeCheck();
                EditorGUILayout.PropertyField(so.FindProperty("m_baseFrame"));
                if (EditorGUI.EndChangeCheck())
                    resetTimeAnimation();

                EditorGUILayout.PropertyField(so.FindProperty("m_frame"));
            }

            // preload
            {
                SerializedProperty preloadLength = so.FindProperty("m_preloadLength");
                preloadLength.intValue = EditorGUILayout.IntSlider("Preload Length", preloadLength.intValue, 0, m_sceneCachePlayer.frameCount);
            }

            EditorGUILayout.Space();
        }
    }

//----------------------------------------------------------------------------------------------------------------------

    void OnFilePathChanged(SceneCachePlayer cachePlayer, string path) {
        cachePlayer.SetFilePath(path);
        Undo.RecordObject(cachePlayer, "SceneCachePlayer");
        cachePlayer.OpenCache(path);
        Repaint();        
    }
    

//----------------------------------------------------------------------------------------------------------------------

    private SceneCachePlayer m_sceneCachePlayer = null;



}

} //end namespace
