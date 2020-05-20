using System;
using System.Collections.Generic;
using Unity.MeshSync;
using UnityEditor;
using UnityEngine;

namespace UnityEditor.MeshSync
{
    [CustomEditor(typeof(MeshSyncPlayer))]
    internal class MeshSyncPlayerEditor : Editor
    {
        private float m_animationFrameRate = 30.0f;
        private float m_animationTimeScale = 1.0f;
        private float m_animationTimeOffset = 0.0f;
        private int m_animationDropStep = 2;
        private float m_reductionThreshold = 0.001f;
        private bool m_eraseFlatCurves = false;

        public virtual void OnEnable()
        {
            var t = target as MeshSyncPlayer;
            var clips = t.GetAnimationClips();
            if (clips.Count > 0)
                m_animationFrameRate = clips[0].frameRate;
        }


        public static void DrawPlayerSettings(MeshSyncPlayer t, SerializedObject so)
        {
            var styleFold = EditorStyles.foldout;
            styleFold.fontStyle = FontStyle.Bold;

            // Sync Settings
            t.foldSyncSettings = EditorGUILayout.Foldout(t.foldSyncSettings, "Sync Settings", true, styleFold);
            if (t.foldSyncSettings)
            {
                EditorGUILayout.PropertyField(so.FindProperty("m_syncVisibility"), new GUIContent("Visibility"));

                EditorGUILayout.PropertyField(so.FindProperty("m_syncTransform"), new GUIContent("Transform"));
                EditorGUILayout.PropertyField(so.FindProperty("m_syncCameras"), new GUIContent("Cameras"));
                if (t.GetSyncSettings().SyncCameras)
                {
                    EditorGUI.indentLevel++;
#if UNITY_2018_1_OR_NEWER
                    EditorGUILayout.PropertyField(so.FindProperty("m_usePhysicalCameraParams"), new GUIContent("Physical Camera Params"));
#endif
                    //EditorGUILayout.PropertyField(so.FindProperty("m_useCustomCameraMatrices"), new GUIContent("Custom View/Proj Matrices"));
                    EditorGUI.indentLevel--;
                }
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
            }

            // Import Settings
            t.foldImportSettings = EditorGUILayout.Foldout(t.foldImportSettings, "Import Settings", true, styleFold);
            if (t.foldImportSettings)
            {
                EditorGUILayout.PropertyField(so.FindProperty("m_animationInterpolation"));
                EditorGUILayout.PropertyField(so.FindProperty("m_keyframeReduction"));
                if (t.keyframeReduction)
                {
                    EditorGUI.indentLevel++;
                    EditorGUILayout.PropertyField(so.FindProperty("m_reductionThreshold"), new GUIContent("Threshold"));
                    EditorGUILayout.PropertyField(so.FindProperty("m_reductionEraseFlatCurves"), new GUIContent("Erase Flat Curves"));
                    EditorGUI.indentLevel--;
                }
                EditorGUILayout.PropertyField(so.FindProperty("m_zUpCorrection"), new GUIContent("Z-Up Correction"));
                EditorGUILayout.Space();
            }

            // Misc
            t.foldMisc = EditorGUILayout.Foldout(t.foldMisc, "Misc", true, styleFold);
            if (t.foldMisc)
            {
                EditorGUILayout.PropertyField(so.FindProperty("m_syncMaterialList"));
                EditorGUILayout.PropertyField(so.FindProperty("m_progressiveDisplay"));
                EditorGUILayout.PropertyField(so.FindProperty("m_logging"));
                EditorGUILayout.PropertyField(so.FindProperty("m_profiling"));
                EditorGUILayout.Space();
            }
        }

        public static void DrawMaterialList(MeshSyncPlayer t, bool allowFold = true)
        {
            Action drawInExportButton = () =>
            {
                GUILayout.BeginHorizontal();
                if (GUILayout.Button("Import List", GUILayout.Width(110.0f)))
                {
                    var path = EditorUtility.OpenFilePanel("Import material list", "Assets", "asset");
                    t.ImportMaterialList(path);
                }
                if (GUILayout.Button("Export List", GUILayout.Width(110.0f)))
                {
                    var path = EditorUtility.SaveFilePanel("Export material list", "Assets", t.name + "_MaterialList", "asset");
                    t.ExportMaterialList(path);
                }
                GUILayout.EndHorizontal();
            };

            if (allowFold)
            {
                var styleFold = EditorStyles.foldout;
                styleFold.fontStyle = FontStyle.Bold;
                t.foldMaterialList = EditorGUILayout.Foldout(t.foldMaterialList, "Materials", true, styleFold);
                if (t.foldMaterialList)
                {
                    DrawMaterialListElements(t);
                    drawInExportButton();
                    if (GUILayout.Button("Open Material Window", GUILayout.Width(160.0f)))
                        MaterialWindow.Open(t);
                    EditorGUILayout.Space();
                }
            }
            else
            {
                GUILayout.Label("Materials", EditorStyles.boldLabel);
                DrawMaterialListElements(t);
                drawInExportButton();
            }
        }

        static void DrawMaterialListElements(MeshSyncPlayer t)
        {
            // calculate label width
            float labelWidth = 60; // minimum
            {
                var style = GUI.skin.box;
                foreach (var md in t.materialList)
                {
                    var size = style.CalcSize(new GUIContent(md.name));
                    labelWidth = Mathf.Max(labelWidth, size.x);
                }
                // 100: margin for color and material field
                labelWidth = Mathf.Min(labelWidth, EditorGUIUtility.currentViewWidth - 100);
            }

            foreach (var md in t.materialList)
            {
                var rect = EditorGUILayout.BeginHorizontal();
                EditorGUI.DrawRect(new Rect(rect.x, rect.y, 16, 16), md.color);
                EditorGUILayout.LabelField("", GUILayout.Width(16));
                EditorGUILayout.LabelField(md.name, GUILayout.Width(labelWidth));
                {
                    var tmp = EditorGUILayout.ObjectField(md.material, typeof(Material), true) as Material;
                    if (tmp != md.material)
                        t.AssignMaterial(md, tmp);
                }
                EditorGUILayout.EndHorizontal();
            }
        }

        public static void DrawTextureList(MeshSyncPlayer t)
        {

        }


        public void DrawAnimationTweak(MeshSyncPlayer t)
        {
            var styleFold = EditorStyles.foldout;
            styleFold.fontStyle = FontStyle.Bold;
            t.foldAnimationTweak = EditorGUILayout.Foldout(t.foldAnimationTweak, "Animation Tweak", true, styleFold);
            if (t.foldAnimationTweak)
            {
                // Override Frame Rate
                GUILayout.BeginVertical("Box");
                EditorGUILayout.LabelField("Override Frame Rate", EditorStyles.boldLabel);
                EditorGUI.indentLevel++;
                m_animationFrameRate = EditorGUILayout.FloatField("Frame Rate", m_animationFrameRate);
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("Apply", GUILayout.Width(120.0f)))
                    ApplyFrameRate(t.GetAnimationClips(), m_animationFrameRate);
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
                GUILayout.EndVertical();

                // Time Scale
                GUILayout.BeginVertical("Box");
                EditorGUILayout.LabelField("Time Scale", EditorStyles.boldLabel);
                EditorGUI.indentLevel++;
                m_animationTimeScale = EditorGUILayout.FloatField("Scale", m_animationTimeScale);
                m_animationTimeOffset = EditorGUILayout.FloatField("Offset", m_animationTimeOffset);
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("Apply", GUILayout.Width(120.0f)))
                    ApplyTimeScale(t.GetAnimationClips(), m_animationTimeScale, m_animationTimeOffset);
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
                GUILayout.EndVertical();

                // Drop Keyframes
                GUILayout.BeginVertical("Box");
                EditorGUILayout.LabelField("Drop Keyframes", EditorStyles.boldLabel);
                EditorGUI.indentLevel++;
                m_animationDropStep = EditorGUILayout.IntField("Step", m_animationDropStep);
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("Apply", GUILayout.Width(120.0f)))
                    ApplyDropKeyframes(t.GetAnimationClips(), m_animationDropStep);
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
                GUILayout.EndVertical();

                // Keyframe Reduction
                GUILayout.BeginVertical("Box");
                EditorGUILayout.LabelField("Keyframe Reduction", EditorStyles.boldLabel);
                EditorGUI.indentLevel++;
                m_reductionThreshold = EditorGUILayout.FloatField("Threshold", m_reductionThreshold);
                m_eraseFlatCurves = EditorGUILayout.Toggle("Erase Flat Curves", m_eraseFlatCurves);
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("Apply", GUILayout.Width(120.0f)))
                    ApplyKeyframeReduction(t.GetAnimationClips(), m_reductionThreshold, m_eraseFlatCurves);
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
                GUILayout.EndVertical();

                EditorGUILayout.Space();
            }
        }


        public void ApplyFrameRate(IEnumerable<AnimationClip> clips, float frameRate)
        {
            foreach (var clip in clips)
            {
                Undo.RegisterCompleteObjectUndo(clip, "ApplyFrameRate");
                clip.frameRate = frameRate;

                Debug.Log("Applied frame rate to " + AssetDatabase.GetAssetPath(clip));
            }
            // repaint animation window
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        }

        public void ApplyTimeScale(IEnumerable<AnimationClip> clips, float timeScale, float timeOffset)
        {
            foreach (var clip in clips)
            {
                var curves = new List<AnimationCurve>();
                var bindings = new List<EditorCurveBinding>();
                var events = AnimationUtility.GetAnimationEvents(clip);

                // gather curves
                bindings.AddRange(AnimationUtility.GetCurveBindings(clip));
                bindings.AddRange(AnimationUtility.GetObjectReferenceCurveBindings(clip));
                foreach (var b in bindings)
                    curves.Add(AnimationUtility.GetEditorCurve(clip, b));

                int eventCount = events.Length;
                int curveCount = curves.Count;

                // transform keys/events
                foreach (var curve in curves)
                {
                    var keys = curve.keys;
                    var keyCount = keys.Length;
                    for (int ki = 0; ki < keyCount; ++ki)
                        keys[ki].time = keys[ki].time * timeScale + timeOffset;
                    curve.keys = keys;
                }
                for (int ei = 0; ei < eventCount; ++ei)
                    events[ei].time = events[ei].time * timeScale + timeOffset;

                // apply changes to clip
                Undo.RegisterCompleteObjectUndo(clip, "ApplyTimeScale");
                clip.frameRate = clip.frameRate / timeScale;
                clip.events = events;
                for (int ci = 0; ci < curveCount; ++ci)
                    Misc.SetCurve(clip, bindings[ci], curves[ci]);

                Debug.Log("Applied time scale to " + AssetDatabase.GetAssetPath(clip));
            }

            // reset m_animationFrameRate
            OnEnable();

            // repaint animation window
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        }

        public void ApplyDropKeyframes(IEnumerable<AnimationClip> clips, int step)
        {
            if (step <= 1)
                return;

            foreach (var clip in clips)
            {
                var curves = new List<AnimationCurve>();
                var bindings = new List<EditorCurveBinding>();

                // gather curves
                bindings.AddRange(AnimationUtility.GetCurveBindings(clip));
                bindings.AddRange(AnimationUtility.GetObjectReferenceCurveBindings(clip));
                foreach (var b in bindings)
                    curves.Add(AnimationUtility.GetEditorCurve(clip, b));

                int curveCount = curves.Count;

                // transform keys/events
                foreach (var curve in curves)
                {
                    var keys = curve.keys;
                    var keyCount = keys.Length;
                    var newKeys = new List<Keyframe>();
                    for (int ki = 0; ki < keyCount; ki += step)
                        newKeys.Add(keys[ki]);
                    curve.keys = newKeys.ToArray();
                }
 
                // apply changes to clip
                Undo.RegisterCompleteObjectUndo(clip, "ApplyDropKeyframes");
                for (int ci = 0; ci < curveCount; ++ci)
                    Misc.SetCurve(clip, bindings[ci], curves[ci]);

                Debug.Log("Applied drop keyframes to " + AssetDatabase.GetAssetPath(clip));
            }

            // repaint animation window
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        }

        public void ApplyKeyframeReduction(IEnumerable<AnimationClip> clips, float threshold, bool eraseFlatCurves)
        {
            foreach (var clip in clips)
            {
                var curves = new List<AnimationCurve>();
                var bindings = new List<EditorCurveBinding>();

                // gather curves
                bindings.AddRange(AnimationUtility.GetCurveBindings(clip));
                bindings.AddRange(AnimationUtility.GetObjectReferenceCurveBindings(clip));
                foreach (var b in bindings)
                    curves.Add(AnimationUtility.GetEditorCurve(clip, b));

                int curveCount = curves.Count;

                // transform keys/events
                foreach (var curve in curves)
                    Misc.KeyframeReduction(curve, threshold, eraseFlatCurves);

                // apply changes to clip
                Undo.RegisterCompleteObjectUndo(clip, "ApplyKeyframeReduction");
                for (int ci = 0; ci < curveCount; ++ci)
                    Misc.SetCurve(clip, bindings[ci], curves[ci]);

                Debug.Log("Applied keyframe reduction to " + AssetDatabase.GetAssetPath(clip));
            }

            // repaint animation window
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        }


        public static void DrawExportAssets(MeshSyncPlayer t)
        {
            var style = EditorStyles.foldout;
            style.fontStyle = FontStyle.Bold;
            t.foldExportAssets = EditorGUILayout.Foldout(t.foldExportAssets, "Export Assets", true, style);
            if (t.foldExportAssets)
            {
                GUILayout.BeginHorizontal();
                if (GUILayout.Button("Export Meshes", GUILayout.Width(160.0f)))
                    t.ExportMeshes();

                if (GUILayout.Button("Export Materials", GUILayout.Width(160.0f)))
                    t.ExportMaterials();
                GUILayout.EndHorizontal();
            }
            EditorGUILayout.Space();
        }

        public static void DrawPluginVersion()
        {
            EditorGUILayout.LabelField("Plugin Version: " + MeshSyncPlayer.GetPluginVersion());
        }
    }
}
