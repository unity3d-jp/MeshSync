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

        SceneCachePlayer scPlayer = clipData.GetSceneCachePlayer();
        if (null == scPlayer)
            return;

        DrawLimitedAnimationGUI(clipData);
        
        {
            // Curve Operations
            GUILayout.BeginVertical("Box");
            EditorGUILayout.LabelField("Curves", EditorStyles.boldLabel);

            const float BUTTON_X     = 30;
            const float BUTTON_WIDTH = 160f;
            if (DrawGUIButton(BUTTON_X, BUTTON_WIDTH,"To Linear")) {
                clipData.SetCurveToLinear();
            }
            
            if (DrawGUIButton(BUTTON_X, BUTTON_WIDTH,"Apply Original")) {
                clipData.ApplyOriginalSceneCacheCurve();
            }
            
            GUILayout.EndVertical();                    
        }
        
        so.ApplyModifiedProperties();       
    }

//----------------------------------------------------------------------------------------------------------------------
    private void DrawLimitedAnimationGUI(SceneCacheClipData clipData) {
        SceneCachePlayer scPlayer = clipData.GetSceneCachePlayer();
        
        bool disableScope = null == scPlayer;
        
        if (null != scPlayer) {
            LimitedAnimationController scLimitedAnimationController = scPlayer.GetLimitedAnimationController();
            bool                       limitedAnimationEnabled      = scLimitedAnimationController.IsEnabled(); 
            if (limitedAnimationEnabled) {
                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.HelpBox("Disable Limited Animation settings on the SceneCache GameObject to control them via Timeline", MessageType.Warning);
                if (GUILayout.Button("Fix", GUILayout.Width(64), GUILayout.Height(36))) {
                    scLimitedAnimationController.SetEnabled(false);
                    Repaint();
                }
                EditorGUILayout.EndHorizontal();
            }

            disableScope |= limitedAnimationEnabled;
        }

        using (new EditorGUI.DisabledScope(disableScope)) {
            SceneCachePlayerEditorUtility.DrawLimitedAnimationGUI(clipData.GetOverrideLimitedAnimationController(),
                m_scPlayableAsset, scPlayer);
        }
        
    }

//----------------------------------------------------------------------------------------------------------------------
    private bool DrawGUIButton(float leftX, float width, string buttonText) {
        Rect rect = GUILayoutUtility.GetRect(new GUIContent("Button"),GUI.skin.button, GUILayout.Width(width));
        rect.x += leftX;
        return (GUI.Button(rect, buttonText, GUI.skin.button));        
    }

//----------------------------------------------------------------------------------------------------------------------


    private static readonly GUIContent SCENE_CACHE_PLAYER = EditorGUIUtility.TrTextContent("Scene Cache Player");
    
    private SceneCachePlayableAsset m_scPlayableAsset;        
    
//----------------------------------------------------------------------------------------------------------------------
    private static class Contents {
        public static readonly GUIContent SnapToFrame = EditorGUIUtility.TrTextContent("Snap To Frame");
    }
    

}
} //end namespace