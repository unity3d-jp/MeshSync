using System;
using System.Collections.Generic;
using Unity.MeshSync;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor
{
    [CustomEditor(typeof(MeshSyncPlayer))]
    internal class MeshSyncPlayerInspector : UnityEditor.Editor {
                
        private float m_animationFrameRate = 30.0f;
       
//----------------------------------------------------------------------------------------------------------------------        
        public virtual void OnEnable() {
            m_asset = target as MeshSyncPlayer;
            
            if (null == m_asset)
                return;
           
            List<AnimationClip> clips = m_asset.GetAnimationClips();
            if (clips.Count > 0)
                m_animationFrameRate = clips[0].frameRate;
        }
        
//----------------------------------------------------------------------------------------------------------------------

        static void EditorGUIToggle(string label, ref bool src) {
            src = EditorGUILayout.Toggle(label, src);
        }

        static void EditorGUIIntField(string label, ref int src) {
            src = EditorGUILayout.IntField(label, src);
        }
        static void EditorGUIFloatField(string label, ref float src) {
            src = EditorGUILayout.FloatField(label, src);
        }
  
        static void EditorGUIPopup(GUIContent content, string[] options, ref int src) {
            src = EditorGUILayout.Popup(content, src, options);                
        }
//----------------------------------------------------------------------------------------------------------------------
        
        protected void DrawPlayerSettings(MeshSyncPlayer t, SerializedObject so)
        {
            var styleFold = EditorStyles.foldout;
            styleFold.fontStyle = FontStyle.Bold;

            // Asset Sync Settings
            t.foldSyncSettings = EditorGUILayout.Foldout(t.foldSyncSettings, "Asset Sync Settings", true, styleFold);
            MeshSyncPlayerConfig playerConfig = m_asset.GetConfig();
            if (t.foldSyncSettings) {

                EditorGUIToggle("Visibility", ref playerConfig.SyncVisibility );
                EditorGUIToggle("Transform", ref playerConfig.SyncTransform );
                EditorGUIToggle("Cameras", ref playerConfig.SyncCameras );

                if (playerConfig.SyncCameras)
                {
                    EditorGUI.indentLevel++;
                    EditorGUILayout.PropertyField(so.FindProperty("m_usePhysicalCameraParams"), new GUIContent("Physical Camera Params"));
                    //EditorGUILayout.PropertyField(so.FindProperty("m_useCustomCameraMatrices"), new GUIContent("Custom View/Proj Matrices"));
                    EditorGUI.indentLevel--;
                }
                EditorGUIToggle("Lights", ref playerConfig.SyncLights );
                EditorGUIToggle("Meshes", ref playerConfig.SyncMeshes );

                EditorGUI.indentLevel++;
                EditorGUIToggle("Update Mesh Colliders", ref playerConfig.UpdateMeshColliders );
                EditorGUI.indentLevel--;

                //EditorGUILayout.PropertyField(so.FindProperty("m_syncPoints"), new GUIContent("Points"));
                EditorGUIToggle("Materials", ref playerConfig.SyncMaterials );
                EditorGUI.indentLevel++;
                EditorGUIToggle("Find From AssetDatabase", ref playerConfig.FindMaterialFromAssets );
                EditorGUI.indentLevel--;

                EditorGUILayout.Space();
            }

            // Import Settings
            t.foldImportSettings = EditorGUILayout.Foldout(t.foldImportSettings, "Import Settings", true, styleFold);
            if (t.foldImportSettings)
            {
                EditorGUIPopup(new GUIContent("Animation Interpolation"), 
                    m_animationInterpolationEnums, ref playerConfig.AnimationInterpolation
                );
                EditorGUIToggle("Keyframe Reduction", ref playerConfig.KeyframeReduction );
                if (playerConfig.KeyframeReduction)
                {
                    EditorGUI.indentLevel++;
                    EditorGUIFloatField("Threshold",ref playerConfig.ReductionThreshold);
                    EditorGUIToggle("Erase Flat Curves", ref playerConfig.ReductionEraseFlatCurves );
                    EditorGUI.indentLevel--;
                }
                EditorGUIPopup(new GUIContent("Z-Up Correction"),m_zUpCorrectionEnums, ref playerConfig.ZUpCorrection);
                EditorGUILayout.Space();
            }

            // Misc
            t.foldMisc = EditorGUILayout.Foldout(t.foldMisc, "Misc", true, styleFold);
            if (t.foldMisc)
            {
                EditorGUIToggle("Sync Material List", ref playerConfig.SyncMaterialList );
                EditorGUIToggle("Progressive Display", ref playerConfig.ProgressiveDisplay );
                EditorGUIToggle("Logging", ref playerConfig.Logging );
                EditorGUIToggle("Profiling", ref playerConfig.Profiling );
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
            if (t.foldAnimationTweak) {
                MeshSyncPlayerConfig config = m_asset.GetConfig();
                AnimationTweakSettings animationTweakSettings = config.GetAnimationTweakSettings();
                    
                
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
                EditorGUIFloatField("Scale", ref animationTweakSettings.TimeScale);
                EditorGUIFloatField("Offset", ref animationTweakSettings.TimeOffset);
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("Apply", GUILayout.Width(120.0f))) {
                    ApplyTimeScale(t.GetAnimationClips(), animationTweakSettings.TimeScale, 
                        animationTweakSettings.TimeOffset
                    );                   
                }
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
                GUILayout.EndVertical();

                // Drop Keyframes
                GUILayout.BeginVertical("Box");
                EditorGUILayout.LabelField("Drop Keyframes", EditorStyles.boldLabel);
                EditorGUI.indentLevel++;
                EditorGUIIntField("Step", ref animationTweakSettings.DropStep);
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("Apply", GUILayout.Width(120.0f))) {
                    ApplyDropKeyframes(t.GetAnimationClips(), animationTweakSettings.DropStep);                    
                }
                GUILayout.EndHorizontal();
                EditorGUI.indentLevel--;
                GUILayout.EndVertical();

                // Keyframe Reduction
                GUILayout.BeginVertical("Box");
                EditorGUILayout.LabelField("Keyframe Reduction", EditorStyles.boldLabel);
                EditorGUI.indentLevel++;
                EditorGUIFloatField("Threshold", ref animationTweakSettings.ReductionThreshold);
                EditorGUIToggle("Erase Flat Curves", ref animationTweakSettings.EraseFlatCurves);
                GUILayout.BeginHorizontal();
                GUILayout.FlexibleSpace();
                if (GUILayout.Button("Apply", GUILayout.Width(120.0f))) {
                    ApplyKeyframeReduction(t.GetAnimationClips(), animationTweakSettings.ReductionThreshold, 
                        animationTweakSettings.EraseFlatCurves
                    );                    
                }
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

        public static void DrawPluginVersion() {
            EditorGUILayout.LabelField("Plugin Version: " + MeshSyncPlayer.GetPluginVersion());
        }

        
//----------------------------------------------------------------------------------------------------------------------

        private MeshSyncPlayer m_asset = null;
        private readonly string[] m_animationInterpolationEnums = System.Enum.GetNames( typeof( InterpolationMode ) );
        private readonly string[] m_zUpCorrectionEnums          = System.Enum.GetNames( typeof( ZUpCorrectionMode ) );

    }
} // end namespace
