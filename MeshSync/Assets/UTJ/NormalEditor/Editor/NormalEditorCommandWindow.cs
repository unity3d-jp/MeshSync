using UnityEngine;
using UnityEditor;

namespace UTJ.HumbleNormalEditor
{
    public class NormalEditorCommandWindow : EditorWindow
    {
        public static bool isOpen;
        Vector2 m_scrollPos;
        NormalEditor m_target;
        Transform m_pivot;
        int m_command;

        Vector3 setValue = Vector3.up;
        Vector3 moveAmount;
        bool rotateUsePivot;
        Vector3 rotateAmount;
        Vector3 scaleAmount;
        float equalizeRadius = 0.5f;
        float equalizeAmount = 1.0f;
        GameObject projector;

        static readonly string[] strCommands = new string[] {
            "Set",
            "Move",
            "Rotate",
            "Scale",
            "Equalize",
            "Projection",
            "Reset",
        };


        [MenuItem("Window/Normal Editor Commands")]
        public static void Open()
        {
            var window = EditorWindow.GetWindow<NormalEditorCommandWindow>();
            window.titleContent = new GUIContent("Normal Commands");
            window.Show();
        }

        void RepaintAllViews()
        {
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        }

        private void OnEnable()
        {
            isOpen = true;
        }

        private void OnDisable()
        {
            isOpen = false;
        }

        private void OnGUI()
        {
            m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos);

            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.BeginVertical(GUILayout.Width(100));
            m_command = GUILayout.SelectionGrid(m_command, strCommands, 1);
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUILayout.Width(10));
            EditorGUILayout.Space();
            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical();

            if (m_command == 0)
            {
                setValue = EditorGUILayout.Vector3Field("Value", setValue);
                if (GUILayout.Button("Set") && m_target != null)
                {
                    m_target.ApplySet(setValue);
                    RepaintAllViews();
                }
            }
            else if (m_command == 1)
            {
                moveAmount = EditorGUILayout.Vector3Field("Move Amount", moveAmount);
                if (GUILayout.Button("Move") && m_target != null)
                {
                    m_target.ApplyMove(moveAmount);
                    RepaintAllViews();
                }
            }
            else if (m_command == 2)
            {
                rotateAmount = EditorGUILayout.Vector3Field("Rotate Amount", rotateAmount);
                if (GUILayout.Button("Rotate") && m_target != null)
                {
                    m_target.ApplyRotate(Quaternion.Euler(rotateAmount.x, rotateAmount.y, rotateAmount.z));
                    RepaintAllViews();
                }
            }
            else if (m_command == 3)
            {
                scaleAmount = EditorGUILayout.Vector3Field("Scale Amount", scaleAmount);
                if (GUILayout.Button("Scale") && m_target != null && m_pivot != null)
                {
                    m_target.ApplyScale(scaleAmount, m_pivot.position);
                    RepaintAllViews();
                }
            }
            else if (m_command == 4)
            {
                equalizeRadius = EditorGUILayout.FloatField("Equalize Radius", equalizeRadius);
                equalizeAmount = EditorGUILayout.FloatField("Equalize Amount", equalizeAmount);
                if (GUILayout.Button("Equalize") && m_target != null)
                {
                    m_target.ApplyEqualize(equalizeRadius, equalizeAmount);
                    RepaintAllViews();
                }
            }
            else if (m_command == 5)
            {
                projector = EditorGUILayout.ObjectField("Projector", projector, typeof(GameObject), true) as GameObject;
                if (GUILayout.Button("Project") && m_target != null)
                {
                    m_target.ApplyProjection(projector);
                    RepaintAllViews();
                }
            }
            else if (m_command == 6)
            {
                if (GUILayout.Button("Reset") && m_target != null)
                {
                    m_target.ResetNormals(false);
                    RepaintAllViews();
                }
            }
            EditorGUILayout.EndVertical();
            EditorGUILayout.EndHorizontal();



            EditorGUILayout.EndScrollView();
        }

        private void OnSelectionChange()
        {
            m_target = null;
            if (Selection.activeGameObject != null)
            {
                m_target = Selection.activeGameObject.GetComponent<NormalEditor>();
            }
            Repaint();
        }
    }
}
