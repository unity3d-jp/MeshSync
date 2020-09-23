using System;
using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync.Editor  {

internal static class InspectorUtility {    

//----------------------------------------------------------------------------------------------------------------------
    
    //[TODO-sin: 2020-9-23] Move to anime-toolbox
    internal static string ShowFileSelectorGUI(string label, 
        string dialogTitle, 
        string fieldValue, 
        Func<string, string> onValidFolderSelected)
    {

        string newDirPath = null;
        using(new EditorGUILayout.HorizontalScope()) {
            if (!string.IsNullOrEmpty (label)) {
                EditorGUILayout.PrefixLabel(label);
            } 

            EditorGUILayout.SelectableLabel(fieldValue,
                EditorStyles.textField, GUILayout.Height(EditorGUIUtility.singleLineHeight)
            );

            //Drag drop
            Rect folderRect = GUILayoutUtility.GetLastRect();
        
            Event evt = Event.current;
            switch (evt.type) {
                case EventType.DragUpdated:
                case EventType.DragPerform:
                    if (!folderRect.Contains (evt.mousePosition))
                        return fieldValue;
     
                    DragAndDrop.visualMode = DragAndDropVisualMode.Copy;
                    if (evt.type == EventType.DragPerform) {
                        DragAndDrop.AcceptDrag ();
    
                        if (DragAndDrop.paths.Length <= 0)
                            break;
                        fieldValue = DragAndDrop.paths[0];
//                            onDragAndDrop(DragAndDrop.paths[0]);
                    }

                    break;
                default:
                    break;
            }
                
            
            newDirPath = InspectorUtility.ShowSelectFileButton(dialogTitle, fieldValue, onValidFolderSelected);

            if (GUILayout.Button("Show", GUILayout.Width(50f),GUILayout.Height(EditorGUIUtility.singleLineHeight))) {
                EditorUtility.RevealInFinder(newDirPath);
            }
            
        }
        
        using (new EditorGUILayout.HorizontalScope()) {
            GUILayout.FlexibleSpace();
            EditorGUI.BeginDisabledGroup(!AssetDatabase.IsValidFolder(newDirPath));        
            if(GUILayout.Button("Highlight in Project Window", GUILayout.Width(180f))) {
                AssetEditorUtility.PingAssetByPath(newDirPath);
            }                
            EditorGUI.EndDisabledGroup();
        }
        
        return newDirPath;
    }
    
    
//----------------------------------------------------------------------------------------------------------------------
    
    private static string ShowSelectFileButton(string title, string folderPath, Func<string, string> onValidFolderSelected) {

        Texture folderTex     = EditorGUIUtility.Load("d_Project@2x") as Texture2D;;
        bool    buttonPressed = false;
        float   lineHeight    = EditorGUIUtility.singleLineHeight;

        if (null == folderTex) {
            buttonPressed = GUILayout.Button("Select", GUILayout.Width(32), GUILayout.Height(lineHeight));            
        } else {
            buttonPressed = GUILayout.Button(folderTex, GUILayout.Width(32), GUILayout.Height(lineHeight));
        }
        if(buttonPressed) {
            string folderSelected = EditorUtility.OpenFilePanel(title, folderPath, "sc");
            if(!string.IsNullOrEmpty(folderSelected)) {
                string newDirPath = null;                    
                if (onValidFolderSelected != null) {
                    newDirPath = onValidFolderSelected (folderSelected);
                } else {
                    newDirPath = folderSelected;
                }

                return newDirPath;
            } else {
                GUIUtility.ExitGUI(); //prevent error when cancel is pressed                
            }
        }

        return folderPath;
    }
    
}

} //end namespace

