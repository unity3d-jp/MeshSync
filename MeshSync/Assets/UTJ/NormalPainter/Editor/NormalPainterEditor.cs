using UnityEngine;
using UnityEditor;
using System.Collections;

namespace UTJ.NormalPainter
{
    [CustomEditor(typeof(NormalPainter))]
    public class NormalPainterEditor : Editor
    {
        public override void OnInspectorGUI()
        {
            if (GUILayout.Button("Open Window"))
                NormalPainterWindow.Open();

            EditorGUILayout.Space();
        }
    }
}
