using System.Collections.Generic;
using System.IO;
using Unity.MeshSync.VariantExport;
using UnityEditor;
using UnityEngine;


namespace Unity.MeshSync.Editor
{
    [CustomEditor(typeof(VariantExporter))]
    internal class VariantExporterEditor : UnityEditor.Editor
    {
        const int BUTTON_WIDTH = 90;
        const int LABEL_WIDTH = 120;

        Transform VariantsHolder
        {
            get
            {
                var exporter = (VariantExporter)target;

                var holder = exporter.transform.Find(exporter.SaveFile);

                return holder;
            }
        }

        public override void OnInspectorGUI()
        {
            var exporter = (VariantExporter)target;

            exporter.Server = (MeshSyncServer)EditorGUILayout.ObjectField("Server", exporter.Server, typeof(MeshSyncServer), true);
            exporter.SaveFile = EditorGUILayout.TextField("Save File", exporter.SaveFile);

            if (exporter.Server)
            {
                DrawProperties(exporter);

                if (GUILayout.Button("Shuffle"))
                {
                    exporter.Shuffle();
                }

                DrawKeepSection(exporter);

                DrawBlockSection(exporter);

                if (!exporter.IsBaking)
                {
                    // exporter.ExportMode = (VariantExporter.ExportModeSetting)EditorGUILayout.EnumPopup(exporter.ExportMode);

                    if (GUILayout.Button("Export!"))
                    {
                        HideVariants();
                        exporter.Export();
                    }
                }
                else
                {
                    if (GUILayout.Button("Stop!"))
                    {
                        exporter.StopExport();
                    }
                }

                EditorGUILayout.LabelField($"Number of exported variants: {exporter.VariantCount}");

                EditorGUILayout.LabelField(new GUIContent($"Number of possible variants: {exporter.TotalPermutationCount}", "Number of possible permutations with all properties"));

                if (Directory.Exists(exporter.SavePath))
                {
                    if (GUILayout.Button("Show created variants"))
                    {
                        ShowVariants();
                    }
                }

                if (VariantsHolder != null && GUILayout.Button("Hide variants"))
                {
                    HideVariants();
                }
            }
        }

        private static void DrawProperties(VariantExporter exporter)
        {
            EditorGUILayout.LabelField("Properties to permutate:");

            foreach (var prop in exporter.Server.propertyInfos)
            {
                if (prop.type != PropertyInfoData.Type.Int)
                {
                    continue;
                }

                GUILayout.BeginHorizontal();
                bool enabled = exporter.IsEnabled(prop);
                //bool newEnabled = EditorGUILayout.Toggle(string.Empty, enabled);
                bool newEnabled = EditorGUILayout.Toggle(enabled, GUILayout.MaxWidth(20));
                if (newEnabled != enabled)
                {
                    if (newEnabled)
                    {
                        exporter.EnabledSettingNames.Add(prop.name);
                    }
                    else
                    {
                        exporter.EnabledSettingNames.Remove(prop.name);
                    }
                }

                MeshSyncServerInspectorUtils.DrawSliderForProperty(prop);

                GUILayout.EndHorizontal();
            }
        }

        private static void DrawBlockSection(VariantExporter exporter)
        {
            GUILayout.BeginHorizontal();
            GUILayout.FlexibleSpace();

            GUILayout.Label($"Blocked variants: {exporter.Blacklist.Count}", GUILayout.Width(LABEL_WIDTH));

            if (exporter.Blacklist.Count > 0 && GUILayout.Button("<", GUILayout.ExpandWidth(false)))
            {
                exporter.PreviousBlockedVariant();
            }

            if (!exporter.IsCurrentVariantBlocked)
            {
                if (GUILayout.Button("Block", GUILayout.Width(BUTTON_WIDTH)))
                {
                    exporter.BlockVariant();
                }
            }
            else
            {
                if (GUILayout.Button("Unblock", GUILayout.Width(BUTTON_WIDTH)))
                {
                    exporter.UnblockVariant();
                }
            }

            if (exporter.Blacklist.Count > 0 && GUILayout.Button(">", GUILayout.ExpandWidth(false)))
            {
                exporter.NextBlockedVariant();
            }

            GUILayout.FlexibleSpace();
            GUILayout.EndHorizontal();
        }

        private static void DrawKeepSection(VariantExporter exporter)
        {
            GUILayout.BeginHorizontal();
            GUILayout.FlexibleSpace();

            GUILayout.Label($"Kept variants: {exporter.Whitelist.Count}", GUILayout.Width(LABEL_WIDTH));

            if (exporter.Whitelist.Count > 0 && GUILayout.Button("<", GUILayout.ExpandWidth(false)))
            {
                exporter.PreviousKeptVariant();
            }

            if (exporter.IsCurrentVariantKept)
            {
                if (GUILayout.Button("Don't keep", GUILayout.Width(BUTTON_WIDTH)))
                {
                    exporter.DoNotKeepVariant();
                }
            }
            else
            {
                if (GUILayout.Button("Keep", GUILayout.Width(BUTTON_WIDTH)))
                {
                    exporter.KeepVariant();
                }
            }

            if (exporter.Whitelist.Count > 0 && GUILayout.Button(">", GUILayout.ExpandWidth(false)))
            {
                exporter.NextKeptVariant();
            }

            GUILayout.FlexibleSpace();
            GUILayout.EndHorizontal();
        }

        private void ShowVariants()
        {
            var exporter = (VariantExporter)target;

            HideVariants();

            var holder = new GameObject(exporter.SaveFile).transform;
            holder.SetParent(exporter.transform, false);
            holder.position = new Vector3(10, 0, 0);

            Vector3 position = Vector3.zero;

            foreach (var prefab in Directory.EnumerateFiles(exporter.SavePath, "*.prefab"))
            {
                var instance = (GameObject)PrefabUtility.InstantiatePrefab(AssetDatabase.LoadAssetAtPath<GameObject>(prefab));
                var renderers = instance.GetComponentsInChildren<Renderer>(true);

                float offset = 0;

                foreach (var renderer in renderers)
                {
                    offset = Mathf.Max(offset, renderer.bounds.size.x / 2);
                }

                offset += 1;

                position.x += offset;

                instance.transform.SetParent(holder, true);
                instance.transform.localPosition = position;
            }

            Selection.activeGameObject = holder.gameObject;
            SceneView.FrameLastActiveSceneView();
        }

        void HideVariants()
        {
            if (VariantsHolder != null)
            {
                DestroyImmediate(VariantsHolder.gameObject);
            }
        }
    }
}
