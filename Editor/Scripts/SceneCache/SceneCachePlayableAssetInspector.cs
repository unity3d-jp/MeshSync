using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {

[CustomEditor(typeof(SceneCachePlayableAsset))]
internal class SceneCachePlayableAssetInspector : UnityEditor.Editor {

    void OnEnable() {
        m_scPlayableAsset = target as SceneCachePlayableAsset;
    }
    
    void OnDisable() {
        m_scPlayableAsset = null;
    }

//----------------------------------------------------------------------------------------------------------------------
    public override void OnInspectorGUI() {
        if (m_scPlayableAsset.IsNullRef())
            return;
        
        SerializedObject so = serializedObject;
        EditorGUILayout.PropertyField(so.FindProperty("m_sceneCachePlayerRef"), SCENE_CACHE_PLAYER);

        SceneCacheClipData clipData = m_scPlayableAsset.GetBoundClipData();
        if (null == clipData)
            return;

        SceneCachePlayer scPlayer = m_scPlayableAsset.GetSceneCachePlayer();
        if (null == scPlayer)
            return;

        DrawLimitedAnimationGUI(m_scPlayableAsset);
        
        {
            // Curve Operations
            GUILayout.BeginVertical("Box");
            EditorGUILayout.LabelField("Curves", EditorStyles.boldLabel);

            const float BUTTON_X     = 30;
            const float BUTTON_WIDTH = 160f;
            if (DrawGUIButton(BUTTON_X, BUTTON_WIDTH,"To Linear")) {
                m_scPlayableAsset.SetCurveToLinearInEditor();
            }
            
            if (DrawGUIButton(BUTTON_X, BUTTON_WIDTH,"Apply Original")) {
                m_scPlayableAsset.ApplyOriginalSceneCacheCurveInEditor();
            }
            
            GUILayout.EndVertical();                    
        }
        
        so.ApplyModifiedProperties();       
    }

//----------------------------------------------------------------------------------------------------------------------
    
    private void DrawLimitedAnimationGUI(SceneCachePlayableAsset scPlayableAsset) {
        SceneCachePlayer scPlayer = scPlayableAsset.GetSceneCachePlayer();
        
        bool disableScope = null == scPlayer;
        
        if (null != scPlayer) {
            disableScope |= (!scPlayer.IsLimitedAnimationOverrideable());
        }

        if (disableScope) {
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.HelpBox(
                "Disabled because Limited Animation settings on the SceneCache GameObject is enabled, or the playback mode is set to interpolate", 
                MessageType.Warning
            );
            if (GUILayout.Button("Fix", GUILayout.Width(64), GUILayout.Height(36))) {
                scPlayer.AllowLimitedAnimationOverride();
                Repaint();
            }
            EditorGUILayout.EndHorizontal();
            
        }

        using (new EditorGUI.DisabledScope(disableScope)) {
            SceneCachePlayerEditorUtility.DrawLimitedAnimationGUI(scPlayableAsset.GetOverrideLimitedAnimationController(),
                m_scPlayableAsset, scPlayer);
        }
        
    }

//----------------------------------------------------------------------------------------------------------------------
    private bool DrawGUIButton(float leftX, float width, string buttonText) {
        Rect rect = UnityEngine.GUILayoutUtility.GetRect(new GUIContent("Button"),GUI.skin.button, GUILayout.Width(width));
        rect.x += leftX;
        return (GUI.Button(rect, buttonText, GUI.skin.button));        
    }

//----------------------------------------------------------------------------------------------------------------------


    private static readonly GUIContent SCENE_CACHE_PLAYER = EditorGUIUtility.TrTextContent("Scene Cache Player");
    
    private SceneCachePlayableAsset m_scPlayableAsset;        
    

}
} //end namespace