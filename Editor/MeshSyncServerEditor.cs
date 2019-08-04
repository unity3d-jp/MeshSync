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
        public static void DrawBaseParams(MeshSyncPlayer t, SerializedObject so)
        {
            EditorGUILayout.PropertyField(so.FindProperty("m_assetDir"));
            EditorGUILayout.PropertyField(so.FindProperty("m_rootObject"));
            EditorGUILayout.Space();

            EditorGUILayout.LabelField("Sync Settings", EditorStyles.boldLabel);
            EditorGUILayout.PropertyField(so.FindProperty("m_syncVisibility"), new GUIContent("Visibility"));
            EditorGUILayout.PropertyField(so.FindProperty("m_syncTransform"), new GUIContent("Transform"));
            EditorGUILayout.PropertyField(so.FindProperty("m_syncCameras"), new GUIContent("Cameras"));
            EditorGUILayout.PropertyField(so.FindProperty("m_syncLights"), new GUIContent("Lights"));
            EditorGUILayout.PropertyField(so.FindProperty("m_syncMeshes"), new GUIContent("Meshes"));
            EditorGUILayout.PropertyField(so.FindProperty("m_syncPoints"), new GUIContent("Points"));
            EditorGUILayout.Space();

            EditorGUILayout.LabelField("Import Settings", EditorStyles.boldLabel);
            EditorGUILayout.PropertyField(so.FindProperty("m_animationInterpolation"));
            EditorGUILayout.PropertyField(so.FindProperty("m_zUpCorrection"));
#if UNITY_2018_1_OR_NEWER
            EditorGUILayout.PropertyField(so.FindProperty("m_usePhysicalCameraParams"));
#endif
            EditorGUILayout.PropertyField(so.FindProperty("m_updateMeshColliders"));
            EditorGUILayout.PropertyField(so.FindProperty("m_findMaterialFromAssets"));
            EditorGUILayout.Space();

            EditorGUILayout.LabelField("Misc", EditorStyles.boldLabel);
            EditorGUILayout.PropertyField(so.FindProperty("m_trackMaterialAssignment"));
            EditorGUILayout.PropertyField(so.FindProperty("m_progressiveDisplay"));
            EditorGUILayout.PropertyField(so.FindProperty("m_logging"));
            EditorGUILayout.Space();
        }

        public static void DrawMaterialList(MeshSyncPlayer t)
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

            GUILayout.Label("Material List", EditorStyles.boldLabel);
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

            DrawBaseParams(t, so);
            DrawMaterialList(t);

            EditorGUILayout.Space();

            //GUILayout.Label("Texture List", EditorStyles.boldLabel);
            //DrawTextureList(t);

            EditorGUILayout.Space();

            if (GUILayout.Button("Open Material Window"))
                MaterialWindow.Open(t);
            EditorGUILayout.Space();

            //if (GUILayout.Button("Generate Lightmap UV"))
            //    t.GenerateLightmapUV();
            //EditorGUILayout.Space();

            if (GUILayout.Button("Export Meshes"))
                t.ExportMeshes();
            EditorGUILayout.Space();

            if (GUILayout.Button("Export Materials"))
                t.ExportMaterials();
            EditorGUILayout.Space();

            EditorGUILayout.LabelField("Plugin Version: " + MeshSyncPlayer.pluginVersion);
            so.ApplyModifiedProperties();
        }
    }
}
