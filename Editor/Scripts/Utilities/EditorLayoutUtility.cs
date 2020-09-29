using System;
using System.IO;
using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync.Editor  {

internal static class EditorLayoutUtility {    

//----------------------------------------------------------------------------------------------------------------------
    
    //[TODO-sin: 2020-9-23] Move to anime-toolbox
    internal static string ShowFileSelectorGUI(string label, 
        string dialogTitle, 
        string fieldValue, 
        Action onReload,
        Func<string, string> onValidFileSelected)
    {

        string newFilePath = null;
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

            if (null != onReload) {
                bool buttonPressed = ShowTextureButton("d_RotateTool On@2x", "Reload");
                if (buttonPressed) {
                    onReload();
                }                
            }           
           
            newFilePath = EditorLayoutUtility.ShowSelectFileButton(dialogTitle, fieldValue, onValidFileSelected);
        }
        
        using (new EditorGUILayout.HorizontalScope()) {
            GUILayout.FlexibleSpace();
            if (GUILayout.Button("Show", GUILayout.Width(50f),GUILayout.Height(EditorGUIUtility.singleLineHeight))) {
                EditorUtility.RevealInFinder(newFilePath);
            }
            bool isValidFile = File.Exists(newFilePath) && newFilePath.StartsWith("Assets/");
            EditorGUI.BeginDisabledGroup(!isValidFile);        
            if(GUILayout.Button("Highlight in Project Window", GUILayout.Width(180f))) {
                AssetEditorUtility.PingAssetByPath(newFilePath);
            }                
            EditorGUI.EndDisabledGroup();
        }
        
        return newFilePath;
    }
    
    

//----------------------------------------------------------------------------------------------------------------------
    private static string ShowSelectFileButton(string title, string folderPath, Func<string, string> onValidFolderSelected) {

        bool    buttonPressed = ShowTextureButton("d_Project@2x", "Select");
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
    
//----------------------------------------------------------------------------------------------------------------------    

    private static bool ShowTextureButton(string textureName, string textFallback, int guiWidth = 32) {

        Texture2D reloadTex     = EditorGUIUtility.Load(textureName) as Texture2D;
        float     lineHeight    = EditorGUIUtility.singleLineHeight;
        if (null == reloadTex) {
            return GUILayout.Button(textFallback, GUILayout.Width(guiWidth), GUILayout.Height(lineHeight));
        }
        
        return GUILayout.Button(reloadTex, GUILayout.Width(guiWidth), GUILayout.Height(lineHeight));
    }
    
}

} //end namespace

