﻿using System.IO;
using NUnit.Framework;
using Unity.MeshSync.Editor;
using UnityEditor.ShortcutManagement;
using UnityEngine;
using UnityEditor;
using UnityEditor.Timeline;
using UnityObject = UnityEngine.Object;


namespace Unity.MeshSync.Editor {

[CustomEditor(typeof(SceneCacheFrameMarker), true)]
[CanEditMultipleObjects]
internal class FrameMarkerInspector: UnityEditor.Editor {

    void OnEnable() {
        int numTargets = targets.Length;
        m_assets = new SceneCacheFrameMarker[numTargets];
        for (int i = 0; i < numTargets; i++) {
            m_assets[i] = targets[i] as SceneCacheFrameMarker;
        }        
    }

//----------------------------------------------------------------------------------------------------------------------
    public override void OnInspectorGUI() {
        ShortcutBinding useFrameShortcut = ShortcutManager.instance.GetShortcutBinding(MeshSyncEditorConstants.SHORTCUT_CHANGE_KEYFRAME_MODE);
        KeyFrameMode    prevMode         = m_assets[0].GetOwner().GetKeyFrameMode();
        KeyFrameMode    mode             = (KeyFrameMode) EditorGUILayout.EnumPopup($"Mode ({useFrameShortcut})", prevMode);
        if (mode != prevMode) {
            foreach (SceneCacheFrameMarker m in m_assets) {
                m.GetOwner().SetKeyFrameMode(mode);
            }
        }

        int prevPlayFrame = m_assets[0].GetOwner().GetPlayFrame();
        int playFrame     = EditorGUILayout.IntField("Frame To Play: ", prevPlayFrame);
        if (prevPlayFrame != playFrame) {
            foreach (SceneCacheFrameMarker m in m_assets) {
                PlayableFrame playableFrame = m.GetOwner();
                playableFrame.SetPlayFrame(playFrame);
            }
            TimelineEditor.Refresh(RefreshReason.ContentsModified);            
        }
        

        if (1 == m_assets.Length) {
            PlayableFrame playableFrame = m_assets[0].GetOwner();
            string           prevNote      = playableFrame?.GetUserNote();            
            DrawNoteGUI(prevNote);            
        } else {

            int numSelectedAssets = m_assets.Length;
            Assert.IsTrue(numSelectedAssets > 1);
            PlayableFrame firstPlayableFrame = m_assets[0].GetOwner();
            //Check invalid PlayableFrame. Perhaps because of unsupported Duplicate operation ?
            if (null == firstPlayableFrame) {
                return;
            }
            string prevNote = firstPlayableFrame.GetUserNote();
            for (int i = 1; i < numSelectedAssets; ++i) {
                PlayableFrame playableFrame = m_assets[i].GetOwner();
                if (playableFrame.GetUserNote() != prevNote) {
                    prevNote = "<different notes>";
                }                                
            }

            DrawNoteGUI(prevNote);

        }
               
    }
       
    
//----------------------------------------------------------------------------------------------------------------------

    internal static void ChangeKeyFrameMode(SceneCacheFrameMarker frameMarker) {
        PlayableFrame playableFrame = frameMarker.GetOwner();
        KeyFrameMode            prevValue     = playableFrame.GetKeyFrameMode();

        playableFrame.SetKeyFrameMode(prevValue == KeyFrameMode.Continuous? KeyFrameMode.Hold : KeyFrameMode.Continuous);
    }
//----------------------------------------------------------------------------------------------------------------------

    private void DrawNoteGUI(string prevNote) {
        GUILayout.Space(15);
        GUILayout.Label("Note");
        
        //Use reflection to access EditorGUI.ScrollableTextAreaInternal()
        Rect rect = EditorGUILayout.GetControlRect(GUILayout.Height(120));
        object[] methodParams = new object[] {
            rect, 
            prevNote, 
            m_noteScroll, 
            EditorStyles.textArea            
        }; 
        object userNoteObj = FilmInternalUtilities.Editor.UnityEditorReflection.SCROLLABLE_TEXT_AREA_METHOD.Invoke(null,methodParams);
        m_noteScroll = (Vector2) (methodParams[2]);
        string userNote = userNoteObj?.ToString();
        
        if (userNote != prevNote) {
            foreach (SceneCacheFrameMarker frameMarker in m_assets) {
                frameMarker.GetOwner().SetUserNote(userNote);
            }
        }
        
    }
    
//----------------------------------------------------------------------------------------------------------------------

    private static void LaunchImageApplicationExternalTool(string imageFullPath) {
        string imageAppPath = EditorPrefs.GetString("kImagesDefaultApp");
        if (string.IsNullOrEmpty(imageAppPath) || !File.Exists(imageAppPath)) {
            System.Diagnostics.Process.Start(imageFullPath);
            return;
        }
        
        System.Diagnostics.Process.Start(imageAppPath, imageFullPath);              
    }
    
    
//----------------------------------------------------------------------------------------------------------------------
    private SceneCacheFrameMarker[] m_assets = null;
    Vector2               m_noteScroll  = Vector2.zero;
    

}

} //end namespace

