﻿using Unity.FilmInternalUtilities;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEditor.Timeline;
using UnityEngine;
using UnityEngine.Timeline;

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

    
    void DrawRegenerateKeyFramesGUI(SceneCachePlayableAsset sceneCachePlayableAsset) 
    {        

        SceneCacheClipData clipData = sceneCachePlayableAsset.GetBoundClipData();
        if (null == clipData)
            return;

        TimelineClip clip = clipData.GetOwner();
        if (null == clip)
            return;

        
        SceneCachePlayableAssetEditorConfig editorConfig = sceneCachePlayableAsset.GetEditorConfig();
        GUILayout.BeginVertical("Box");
        EditorGUILayout.LabelField("Regenerate Key Frames", EditorStyles.boldLabel);

        //UI
        EditorGUI.BeginChangeCheck();
        int keyFrameSpan = EditorGUILayout.IntField("KeyFrame Span", editorConfig.GetGenerateKeyFrameSpan());
        KeyFrameMode keyFrameMode = (KeyFrameMode) EditorGUILayout.EnumPopup("KeyFrame Mode", editorConfig.GetGenerateKeyFrameMode());
            
        bool generateAllKeyFrames = EditorGUILayout.Toggle("All Frames", editorConfig.GetGenerateAllKeyFrames());
        EditorGUI.BeginDisabledGroup(generateAllKeyFrames);
        ++EditorGUI.indentLevel;

        int startKeyFrame = Mathf.Max(0,editorConfig.GetGenerateStartKeyFrame());
        int endKeyFrame   = editorConfig.GetGenerateEndKeyFrame();
        if (endKeyFrame < 0) {
            endKeyFrame = TimelineUtility.CalculateNumFrames(clipData.GetOwner());
        }

        startKeyFrame = EditorGUILayout.IntField("From", startKeyFrame);
        endKeyFrame   = EditorGUILayout.IntField("To", endKeyFrame);

        --EditorGUI.indentLevel;
        EditorGUI.EndDisabledGroup();
            
        if (EditorGUI.EndChangeCheck()) {
            Undo.RecordObject(sceneCachePlayableAsset,"SceneCache: Generate KeyFrames");
            
            editorConfig.SetGenerateKeyFrameSpan(keyFrameSpan);
            editorConfig.SetGenerateKeyFrameMode(keyFrameMode);
            
            editorConfig.SetGenerateAllKeyFrames(generateAllKeyFrames);
            editorConfig.SetGenerateStartKeyFrame(startKeyFrame);
            editorConfig.SetGenerateEndKeyFrame(endKeyFrame);
        }
               
        GUILayout.Space(15);        
        if (DrawGUIButton( leftX:15, width:120,"Generate")) {
            if (generateAllKeyFrames)
                clipData.RegenerateKeyFrames(keyFrameSpan, keyFrameMode);
            else 
                clipData.RegenerateKeyFrames(startKeyFrame, endKeyFrame, keyFrameSpan, keyFrameMode);
        }
            
        GUILayout.EndVertical();
        
        
    }

//----------------------------------------------------------------------------------------------------------------------


    private static readonly GUIContent SCENE_CACHE_PLAYER = EditorGUIUtility.TrTextContent("Scene Cache Player");

    
    private SceneCachePlayableAsset m_scPlayableAsset;
    
    

}
} //end namespace