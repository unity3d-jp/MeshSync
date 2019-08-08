using UnityEditor;
using UnityEngine;
using UTJ.MeshSync;

namespace UTJ.MeshSyncEditor
{
    [CustomEditor(typeof(MeshSyncServer))]
    public class MeshSyncServerEditor : Editor
    {
#if UNITY_EDITOR
        [MenuItem("GameObject/MeshSync/Create Server", false, 10)]
        public static void CreateMeshSyncServer(MenuCommand menuCommand)
        {
            var go = new GameObject();
            go.name = "MeshSyncServer";
            var mss = go.AddComponent<MeshSyncServer>();
            mss.rootObject = go.GetComponent<Transform>();
            Undo.RegisterCreatedObjectUndo(go, "MeshSyncServer");
        }
#endif
        public static void DrawPlayerSettings(MeshSyncPlayer t, SerializedObject so)
        {
            // Sync Settings
            EditorGUILayout.LabelField("Sync Settings", EditorStyles.boldLabel);
            EditorGUILayout.PropertyField(so.FindProperty("m_syncVisibility"), new GUIContent("Visibility"));

            EditorGUILayout.PropertyField(so.FindProperty("m_syncTransform"), new GUIContent("Transform"));
            EditorGUILayout.PropertyField(so.FindProperty("m_syncCameras"), new GUIContent("Cameras"));
#if UNITY_2018_1_OR_NEWER
            if (t.syncCameras)
            {
                EditorGUI.indentLevel++;
                EditorGUILayout.PropertyField(so.FindProperty("m_usePhysicalCameraParams"), new GUIContent("Physical Camera Params"));
                EditorGUI.indentLevel--;
            }
#endif
            EditorGUILayout.PropertyField(so.FindProperty("m_syncLights"), new GUIContent("Lights"));

            EditorGUILayout.PropertyField(so.FindProperty("m_syncMeshes"), new GUIContent("Meshes"));
            EditorGUI.indentLevel++;
            EditorGUILayout.PropertyField(so.FindProperty("m_updateMeshColliders"));
            EditorGUI.indentLevel--;

            //EditorGUILayout.PropertyField(so.FindProperty("m_syncPoints"), new GUIContent("Points"));
            EditorGUILayout.PropertyField(so.FindProperty("m_syncMaterials"), new GUIContent("Materials"));
            EditorGUI.indentLevel++;
            EditorGUILayout.PropertyField(so.FindProperty("m_findMaterialFromAssets"), new GUIContent("Find From AssetDatabase"));
            EditorGUI.indentLevel--;

            EditorGUILayout.Space();

            // Import Settings
            EditorGUILayout.LabelField("Import Settings", EditorStyles.boldLabel);
            EditorGUILayout.PropertyField(so.FindProperty("m_animationInterpolation"));
            EditorGUILayout.PropertyField(so.FindProperty("m_zUpCorrection"), new GUIContent("Z-Up Correction"));
            EditorGUILayout.Space();

            // Misc
            EditorGUILayout.LabelField("Misc", EditorStyles.boldLabel);
            //EditorGUILayout.PropertyField(so.FindProperty("m_trackMaterialAssignment"));
            EditorGUILayout.PropertyField(so.FindProperty("m_progressiveDisplay"));
            EditorGUILayout.PropertyField(so.FindProperty("m_logging"));
            EditorGUILayout.Space();
        }

        public static void DrawMaterialList(MeshSyncPlayer t, bool allowFold = true)
        {
            if (allowFold)
            {
                var style = EditorStyles.foldout;
                style.fontStyle = FontStyle.Bold;
                t.foldMaterialList = EditorGUILayout.Foldout(t.foldMaterialList, "Material List", true, style);
                if (t.foldMaterialList)
                {
                    DrawMaterialListElements(t);
                }
            }
            else
            {
                GUILayout.Label("Material List", EditorStyles.boldLabel);
                DrawMaterialListElements(t);
            }
            if (GUILayout.Button("Open Material Window", GUILayout.Width(160.0f)))
                MaterialWindow.Open(t);
            EditorGUILayout.Space();
        }

        static void DrawMaterialListElements(MeshSyncPlayer t)
        {
            // calculate label width
            float labelWidth = 60; // minimum
            {
                var style = GUI.skin.box;
                foreach (var md in t.materialData)
                {
                    var size = style.CalcSize(new GUIContent(md.name));
                    labelWidth = Mathf.Max(labelWidth, size.x);
                }
                // 100: margin for color and material field
                labelWidth = Mathf.Min(labelWidth, EditorGUIUtility.currentViewWidth - 100);
            }

            foreach (var md in t.materialData)
            {
                var rect = EditorGUILayout.BeginHorizontal();
                EditorGUI.DrawRect(new Rect(rect.x, rect.y, 16, 16), md.color);
                EditorGUILayout.LabelField("", GUILayout.Width(16));
                EditorGUILayout.LabelField(md.name, GUILayout.Width(labelWidth));
                {
                    var tmp = EditorGUILayout.ObjectField(md.material, typeof(Material), true) as Material;
                    if (tmp != md.material)
                    {
                        Undo.RecordObject(t, "MeshSyncServer");
                        md.material = tmp;
                        t.ReassignMaterials();
                        t.ForceRepaint();
                    }
                }
                EditorGUILayout.EndHorizontal();
            }
        }

        public static void DrawTextureList(MeshSyncPlayer t)
        {

        }

        public override void OnInspectorGUI()
        {
            var so = serializedObject;
            var t = target as MeshSyncPlayer;

            // server param
            EditorGUILayout.LabelField("Server", EditorStyles.boldLabel);
            EditorGUILayout.PropertyField(so.FindProperty("m_serverPort"));
            EditorGUILayout.PropertyField(so.FindProperty("m_assetDir"));
            EditorGUILayout.PropertyField(so.FindProperty("m_rootObject"));
            EditorGUILayout.Space();

            MeshSyncServerEditor.DrawPlayerSettings(t, so);
            MeshSyncServerEditor.DrawMaterialList(t);
            MeshSyncServerEditor.DrawTextureList(t);

            if (GUILayout.Button("Export Meshes", GUILayout.Width(160.0f)))
                t.ExportMeshes();
            EditorGUILayout.Space();

            if (GUILayout.Button("Export Materials", GUILayout.Width(160.0f)))
                t.ExportMaterials();
            EditorGUILayout.Space();

            EditorGUILayout.LabelField("Plugin Version: " + MeshSyncPlayer.pluginVersion);
            so.ApplyModifiedProperties();
        }
    }
}
