using System.IO;
using NUnit.Framework;
using Unity.MeshSync.Editor;
using UnityEditor.ShortcutManagement;
using UnityEngine;
using UnityEditor;
using UnityEditor.Timeline;
using UnityObject = UnityEngine.Object;


namespace Unity.MeshSync.Editor {

[CustomEditor(typeof(FrameMarker), true)]
[CanEditMultipleObjects]
internal class FrameMarkerInspector: UnityEditor.Editor {

    void OnEnable() {
        int numTargets = targets.Length;
        m_assets = new FrameMarker[numTargets];
        for (int i = 0; i < numTargets; i++) {
            m_assets[i] = targets[i] as FrameMarker;
        }        
    }

//----------------------------------------------------------------------------------------------------------------------
    public override void OnInspectorGUI() {
        ShortcutBinding useFrameShortcut 
            = ShortcutManager.instance.GetShortcutBinding(MeshSyncEditorConstants.SHORTCUT_TOGGLE_KEYFRAME);
        KeyFrameMode prevMode = m_assets[0].GetKeyFrameMode();
        KeyFrameMode mode     = (KeyFrameMode) EditorGUILayout.EnumPopup($"Mode ({useFrameShortcut})", prevMode);
        if (mode != prevMode) {
            foreach (FrameMarker m in m_assets) {
                SetMarkerValueByContext(m,(int) mode);
            }            
        }

        int prevPlayFrame = m_assets[0].GetOwner().GetFrameNo();
        int playFrame     = EditorGUILayout.IntField("Frame To Play: ", prevPlayFrame);
        if (prevPlayFrame != playFrame) {
            foreach (FrameMarker m in m_assets) {
                SISPlayableFrame playableFrame = m.GetOwner();
                playableFrame.SetPlayFrame(playFrame);
            }
            TimelineEditor.Refresh(RefreshReason.ContentsModified);
            
        }
        

        if (1 == m_assets.Length) {
            SISPlayableFrame playableFrame = m_assets[0].GetOwner();
            string           prevNote      = playableFrame?.GetUserNote();            
            DrawNoteGUI(prevNote);            
        } else {

            int numSelectedAssets = m_assets.Length;
            Assert.IsTrue(numSelectedAssets > 1);
            SISPlayableFrame firstPlayableFrame = m_assets[0].GetOwner();
            //Check invalid PlayableFrame. Perhaps because of unsupported Duplicate operation ?
            if (null == firstPlayableFrame) {
                return;
            }
            string prevNote = firstPlayableFrame.GetUserNote();
            for (int i = 1; i < numSelectedAssets; ++i) {
                SISPlayableFrame playableFrame = m_assets[i].GetOwner();
                if (playableFrame.GetUserNote() != prevNote) {
                    prevNote = "<different notes>";
                }                                
            }

            DrawNoteGUI(prevNote);

        }
               
    }
       
    
//----------------------------------------------------------------------------------------------------------------------
    private static void SetMarkerValueByContext(FrameMarker frameMarker, int value) {
        SISPlayableFrame      playableFrame = frameMarker.GetOwner();
        PlayableFrameClipData clipData      = playableFrame.GetOwner();
        KeyFramePropertyID inspectedPropertyID = clipData.GetInspectedProperty();
        playableFrame.SetProperty(inspectedPropertyID, value);
    }
    

    internal static void ToggleMarkerValueByContext(FrameMarker frameMarker) {
        SISPlayableFrame      playableFrame       = frameMarker.GetOwner();
        PlayableFrameClipData clipData            = playableFrame.GetOwner();
        KeyFramePropertyID    inspectedPropertyID = clipData.GetInspectedProperty();
        int                   prevValue           = playableFrame.GetProperty(inspectedPropertyID);
        
        switch (inspectedPropertyID) {
            case KeyFramePropertyID.Mode: {                
                playableFrame.SetProperty(inspectedPropertyID, prevValue == (int) KeyFrameMode.Continuous ? (int) KeyFrameMode.Hold : (int) KeyFrameMode.Continuous);
                break;
            }

        }
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
            foreach (FrameMarker frameMarker in m_assets) {
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
    private FrameMarker[] m_assets = null;
    Vector2               m_noteScroll  = Vector2.zero;
    

}

} //end namespace

