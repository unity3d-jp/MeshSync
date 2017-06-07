using UnityEngine;
using UnityEditor;
using System.Collections;

namespace UTJ.HumbleNormalEditor
{
    [CustomEditor(typeof(NormalEditor))]
    public class NormalEditorEditor : Editor
    {
        NormalEditor m_target;


        void OnEnable()
        {
            m_target = target as NormalEditor;
        }

        void OnDisable()
        {
        }

        void OnSceneGUI()
        {
            if (!NormalEditorWindow.isOpen)
            {
                int ret = m_target.OnSceneGUI();
                if ((ret & (int)SceneGUIState.Repaint) != 0)
                    Repaint();
            }
        }

        public override void OnInspectorGUI()
        {
            if (GUILayout.Button("Open Editor Window"))
                NormalEditorWindow.Open();

            EditorGUILayout.Space();

            if(!NormalEditorWindow.isOpen)
            {
                NormalEditorWindow.DrawNormalEditor(m_target);
            }
        }
    }
}
