using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEditor.Timeline;
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


        GUILayout.Space(15);
        //Frame markers
        if (null!=TimelineEditor.selectedClip && TimelineEditor.selectedClip.asset == m_scPlayableAsset) {
            DrawRegenerateKeyFramesGUI(m_scPlayableAsset);
        }
        GUILayout.Space(15);
                
        if (DrawGUIButton(leftX:0, width:120,"Reset")) {
            m_scPlayableAsset.InitKeyFrames();
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
                "Disabled because the playback mode of the SceneCache is set to Interpolate", 
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

    
    void DrawRegenerateKeyFramesGUI<T>(BaseExtendedClipPlayableAsset<T> clipDataPlayableAsset) where T: PlayableFrameClipData 
    {        

        T clipData = clipDataPlayableAsset.GetBoundClipData();
        if (null == clipData)
            return;

        GUILayout.BeginVertical("Box");
        EditorGUILayout.LabelField("Regenerate Key Frames", EditorStyles.boldLabel);

        m_autoKeyFrameSpan = EditorGUILayout.IntField("KeyFrame Span", m_autoKeyFrameSpan);
        m_autoKeyFrameMode = (KeyFrameMode) EditorGUILayout.EnumPopup("Mode", m_autoKeyFrameMode);
        
        GUILayout.Space(15);        
        if (DrawGUIButton( leftX:15, width:120,"Generate")) {
            clipData.GenerateKeyFramesAuto(m_autoKeyFrameSpan,m_autoKeyFrameMode);
        }
            
        GUILayout.EndVertical();
        
        
    }
//----------------------------------------------------------------------------------------------------------------------


    private static readonly GUIContent SCENE_CACHE_PLAYER = EditorGUIUtility.TrTextContent("Scene Cache Player");

    private int          m_autoKeyFrameSpan = 3;
    private KeyFrameMode m_autoKeyFrameMode = KeyFrameMode.Continuous;
    
    private SceneCachePlayableAsset m_scPlayableAsset;
    
    

}
} //end namespace