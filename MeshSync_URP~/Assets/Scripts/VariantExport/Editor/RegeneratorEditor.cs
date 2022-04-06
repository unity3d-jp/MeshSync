using System.Collections.Generic;
using System.IO;
using Unity.MeshSync.VariantExport;
using UnityEditor;
using UnityEngine;


namespace Unity.MeshSync.Editor
{
    [CustomEditor(typeof(Regenerator))]
    internal class RegeneratorEditor : UnityEditor.Editor
    {
        const int TOGGLE_WIDTH = 20;
        const int BUTTON_WIDTH = 90;
        const int LABEL_WIDTH = 120;

        Transform VariantsHolder
        {
            get
            {
                var exporter = (Regenerator)target;

                if (string.IsNullOrEmpty(exporter.SaveFile))
                {
                    return null;
                }

                var holder = exporter.transform.Find(exporter.SaveFile);

                return holder;
            }
        }

        public override void OnInspectorGUI()
        {
            var exporter = (Regenerator)target;

            exporter.Server = (MeshSyncServer)EditorGUILayout.ObjectField("Server", exporter.Server, typeof(MeshSyncServer), true);
            exporter.SaveFile = EditorGUILayout.TextField("Save File", exporter.SaveFile);

            if (exporter.Server == null)
            {
                if (GUILayout.Button("Find server"))
                {
                    exporter.Server = FindObjectOfType<MeshSyncServer>();
                }
            }
            else
            {

                if (exporter.Server.propertyInfos.Count == 0)
                {
                    EditorGUILayout.Space();
                    EditorGUILayout.LabelField("Cannot find properties. Is 'Auto-sync' enabled in blender?");
                    EditorGUILayout.Space();

                }
                else
                {
                    DrawProperties(exporter);

                    DrawShuffleSection(exporter);

                    DrawKeepSection(exporter);

                    DrawBlockSection(exporter);

                    if (GUILayout.Button("Clear kept and blocked"))
                    {
                        if (EditorUtility.DisplayDialog("Warning", $"This will clear all kept and blocked variant settings. Are you sure?", "Yes", "No"))
                        {
                            exporter.Clear();
                        }
                    }

                    if (!exporter.IsBaking)
                    {
                        if (string.IsNullOrEmpty(exporter.SaveFile))
                        {
                            EditorGUILayout.LabelField("Save file is not set, cannot export.");
                        }
                        else
                        {
                            if (GUILayout.Button("Regenerate!"))
                            {
                                HideVariants();
                                exporter.Regenerate();
                            }
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

                    EditorGUILayout.LabelField(new GUIContent($"Number of possible variants: {exporter.TotalPermutationCount}", "Number of possible permutations with all properties enabled."));

                    if (!string.IsNullOrEmpty(exporter.SaveFile) && Directory.Exists(exporter.SavePath))
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

                    if (exporter.Server.CurrentPropertiesState == MeshSyncServer.PropertiesState.Sending)
                    {
                        EditorGUILayout.LabelField("Waiting for blender...");
                    }
                }
            }
        }

        private static void DrawShuffleSection(Regenerator exporter)
        {
            EditorGUILayout.Space();

            exporter.AutoShuffle = EditorGUILayout.ToggleLeft("Auto shuffle", exporter.AutoShuffle);

            if (GUILayout.Button("Shuffle"))
            {
                exporter.Shuffle();
            }
        }

        private static void DrawProperties(Regenerator exporter)
        {
            EditorGUILayout.LabelField("Properties to permutate:");

            bool allEnabled = true;
            foreach (var prop in exporter.Server.propertyInfos)
            {
                if (!prop.CanBeModified)
                {
                    continue;
                }

                if (!exporter.IsEnabled(prop))
                {
                    allEnabled = false;
                    break;
                }
            }

            bool newAllEnabled = EditorGUILayout.Toggle(allEnabled, GUILayout.MaxWidth(TOGGLE_WIDTH));
            if (allEnabled != newAllEnabled)
            {
                if (newAllEnabled)
                {
                    foreach (var prop in exporter.Server.propertyInfos)
                    {
                        exporter.EnabledSettingNames.Add(prop.name);
                    }
                }
                else
                {
                    exporter.EnabledSettingNames.Clear();
                }
            }

            foreach (var prop in exporter.Server.propertyInfos)
            {
                if (!prop.CanBeModified)
                {
                    continue;
                }

                GUILayout.BeginHorizontal();
                bool enabled = exporter.IsEnabled(prop);
                bool newEnabled = EditorGUILayout.Toggle(enabled, GUILayout.MaxWidth(TOGGLE_WIDTH));
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

        private static void DrawBlockSection(Regenerator exporter)
        {
            GUILayout.BeginHorizontal();
            GUILayout.FlexibleSpace();

            int currentIndex = exporter.Blacklist.IndexOf(exporter.SerializeCurrentVariant(true));
            if (currentIndex == -1)
            {
                GUILayout.Label($"Blocked variants: {exporter.Blacklist.Count}", GUILayout.Width(LABEL_WIDTH));
            }
            else
            {
                GUILayout.Label($"Blocked variant: {currentIndex + 1}/{exporter.Whitelist.Count}", GUILayout.Width(LABEL_WIDTH));
            }

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

        private static void DrawKeepSection(Regenerator exporter)
        {
            GUILayout.BeginHorizontal();
            GUILayout.FlexibleSpace();

            int currentIndex = exporter.Whitelist.IndexOf(exporter.SerializeCurrentVariant(true));
            if (currentIndex == -1)
            {
                GUILayout.Label($"Kept variants: {exporter.Whitelist.Count}", GUILayout.Width(LABEL_WIDTH));
            }
            else
            {
                GUILayout.Label($"Kept variant: {currentIndex + 1}/{exporter.Whitelist.Count}", GUILayout.Width(LABEL_WIDTH));
            }

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
            var exporter = (Regenerator)target;

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

        [MenuItem("GameObject/MeshSync/Create Regenerator", false, 20)]
        internal static void CreateRegeneratorMenu()
        {
            var go = new GameObject("Regenerator");
            var exporter = go.AddComponent<Regenerator>();
            exporter.Server = FindObjectOfType<MeshSyncServer>();

            Undo.RegisterCreatedObjectUndo(go, "Regenerator");
        }
    }
}
