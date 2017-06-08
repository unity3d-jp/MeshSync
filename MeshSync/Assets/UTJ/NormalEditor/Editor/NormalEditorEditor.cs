using UnityEngine;
using UnityEditor;
using System.Collections;

namespace UTJ.HumbleNormalEditor
{
    [CustomEditor(typeof(NormalEditor))]
    public class NormalEditorEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            if (GUILayout.Button("Open Window"))
                NormalEditorWindow.Open();

            EditorGUILayout.Space();
        }
    }
}
