using System;
using System.Collections.Generic;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;
using Unity.MeshSync;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor
{
    [CustomEditor(typeof(MeshSyncPlayer))]
    internal class MeshSyncPlayerInspector : UnityEditor.Editor {
                
       
//----------------------------------------------------------------------------------------------------------------------        
        public virtual void OnEnable() {
            m_asset = target as MeshSyncPlayer;
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
        
        protected void DrawPlayerSettings(MeshSyncPlayer t)
        {
            var styleFold = EditorStyles.foldout;
            styleFold.fontStyle = FontStyle.Bold;

            // Asset Sync Settings
            t.foldSyncSettings = EditorGUILayout.Foldout(t.foldSyncSettings, "Asset Sync Settings", true, styleFold);
            MeshSyncPlayerConfig playerConfig = m_asset.GetConfig();
            if (t.foldSyncSettings) {

                EditorGUIDrawerUtility.DrawUndoableGUI(target,"MeshSync: Sync Visibility",
                    guiFunc: () => EditorGUILayout.Toggle("Visibility", playerConfig.SyncVisibility), 
                    updateFunc: (bool toggle) => { playerConfig.SyncVisibility = toggle; }
                );

                EditorGUIDrawerUtility.DrawUndoableGUI(target,"MeshSync: Sync Transform",
                    guiFunc: () => EditorGUILayout.Toggle("Transform", playerConfig.SyncTransform), 
                    updateFunc: (bool toggle) => { playerConfig.SyncTransform = toggle; }
                );

                EditorGUIDrawerUtility.DrawUndoableGUI(target,"MeshSync: Sync Cameras",
                    guiFunc: () => EditorGUILayout.Toggle("Cameras", playerConfig.SyncCameras), 
                    updateFunc: (bool toggle) => { playerConfig.SyncCameras = toggle; }
                );
                
                if (playerConfig.SyncCameras)
                {
                    EditorGUI.indentLevel++;
                    
                    t.SetUsePhysicalCameraParams(EditorGUILayout.Toggle("Physical Camera Params", t.GetUsePhysicalCameraParams()));
                    //EditorGUILayout.PropertyField(so.FindProperty("m_useCustomCameraMatrices"), new GUIContent("Custom View/Proj Matrices"));
                    EditorGUI.indentLevel--;
                }
                
                EditorGUIDrawerUtility.DrawUndoableGUI(target,"MeshSync: Sync Lights",
                    guiFunc: () => EditorGUILayout.Toggle("Lights", playerConfig.SyncLights), 
                    updateFunc: (bool toggle) => { playerConfig.SyncLights = toggle; }
                );

                EditorGUIDrawerUtility.DrawUndoableGUI(target,"MeshSync: Sync Meshes",
                    guiFunc: () => EditorGUILayout.Toggle("Meshes", playerConfig.SyncMeshes), 
                    updateFunc: (bool toggle) => { playerConfig.SyncMeshes = toggle; }
                );
                

                EditorGUI.indentLevel++;
                EditorGUIDrawerUtility.DrawUndoableGUI(target,"MeshSync: Update Mesh Colliders",
                    guiFunc: () => EditorGUILayout.Toggle("Update Mesh Colliders", playerConfig.UpdateMeshColliders), 
                    updateFunc: (bool toggle) => { playerConfig.UpdateMeshColliders = toggle; }
                );
                EditorGUI.indentLevel--;

                //EditorGUILayout.PropertyField(so.FindProperty("m_syncPoints"), new GUIContent("Points"));
                EditorGUIDrawerUtility.DrawUndoableGUI(target,"MeshSync: Sync Materials",
                    guiFunc: () => EditorGUILayout.Toggle("Materials", playerConfig.SyncMaterials), 
                    updateFunc: (bool toggle) => { playerConfig.SyncMaterials = toggle; }
                );
                
                EditorGUI.indentLevel++;
                EditorGUIDrawerUtility.DrawUndoableGUI(target,"MeshSync: Find From Asset Database",
                    guiFunc: () => EditorGUILayout.Toggle("Find From AssetDatabase", playerConfig.FindMaterialFromAssets), 
                    updateFunc: (bool toggle) => { playerConfig.FindMaterialFromAssets = toggle; }
                );
                
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

        public static void DrawTextureList(MeshSyncPlayer t) {

        }

//----------------------------------------------------------------------------------------------------------------------        

        protected static void DrawAnimationTweak(MeshSyncPlayer player) {
            GUIStyle styleFold = EditorStyles.foldout;
            styleFold.fontStyle = FontStyle.Bold;
            player.foldAnimationTweak = EditorGUILayout.Foldout(player.foldAnimationTweak, "Animation Tweak", true, styleFold);
            if (player.foldAnimationTweak) {
                MeshSyncPlayerConfig config = player.GetConfig();
                AnimationTweakSettings animationTweakSettings = config.GetAnimationTweakSettings();
                
                float               frameRate = 30.0f;
                List<AnimationClip> clips     = player.GetAnimationClips();
                if (clips.Count > 0) {
                    frameRate = clips[0].frameRate;                    
                }

                {
                    // Override Frame Rate
                    GUILayout.BeginVertical("Box");
                    EditorGUILayout.LabelField("Override Frame Rate", EditorStyles.boldLabel);
                    EditorGUI.indentLevel++;
                    float prevFrameRate = frameRate;
                    frameRate = EditorGUILayout.FloatField("Frame Rate", frameRate);
                    if (!Mathf.Approximately(prevFrameRate, frameRate) && frameRate > 0) {
                        ApplyFrameRate(clips, frameRate);                    
                    }
                    EditorGUI.indentLevel--;
                    GUILayout.EndVertical();                    
                }
                
                

                // Time Scale
                {
                    GUILayout.BeginVertical("Box");
                    EditorGUILayout.LabelField("Time Scale", EditorStyles.boldLabel);
                    EditorGUI.indentLevel++;
                    float prevTimeScale  = animationTweakSettings.TimeScale;
                    float prevTimeOffset = animationTweakSettings.TimeOffset;
                    EditorGUIFloatField("Scale", ref animationTweakSettings.TimeScale);
                    EditorGUIFloatField("Offset", ref animationTweakSettings.TimeOffset);
                    if (!Mathf.Approximately(prevTimeScale, animationTweakSettings.TimeScale) ||
                        !Mathf.Approximately(prevTimeOffset, animationTweakSettings.TimeOffset)
                    ) 
                    {                    
                        ApplyTimeScale(clips, animationTweakSettings.TimeScale, 
                            animationTweakSettings.TimeOffset
                        );                   
                    
                    }                    
                    EditorGUI.indentLevel--;
                    GUILayout.EndVertical();                    
                }

                // Drop Keyframes 
                {
                    GUILayout.BeginVertical("Box");
                    EditorGUILayout.LabelField("Drop Keyframes", EditorStyles.boldLabel);
                    EditorGUI.indentLevel++;
                    int prevDropStep = animationTweakSettings.DropStep;
                    EditorGUIIntField("Step", ref animationTweakSettings.DropStep);
                    if (prevDropStep != animationTweakSettings.DropStep && animationTweakSettings.DropStep > 1) {
                        ApplyDropKeyframes(clips, animationTweakSettings.DropStep);                                        
                    }
                    EditorGUI.indentLevel--;
                    GUILayout.EndVertical();                    
                }

                // Keyframe Reduction
                {
                    GUILayout.BeginVertical("Box");
                    EditorGUILayout.LabelField("Keyframe Reduction", EditorStyles.boldLabel);
                    EditorGUI.indentLevel++;
                    float prevReductionThreshold = animationTweakSettings.ReductionThreshold;
                    bool  prevEraseFlatCurves    = animationTweakSettings.EraseFlatCurves;
                    EditorGUIFloatField("Threshold", ref animationTweakSettings.ReductionThreshold);
                    EditorGUIToggle("Erase Flat Curves", ref animationTweakSettings.EraseFlatCurves);
                    if (!Mathf.Approximately(prevReductionThreshold, animationTweakSettings.ReductionThreshold)
                        || prevEraseFlatCurves!=animationTweakSettings.EraseFlatCurves) 
                    {
                        ApplyKeyframeReduction(clips, animationTweakSettings.ReductionThreshold, 
                            animationTweakSettings.EraseFlatCurves
                        );                                        
                    }               
                    EditorGUI.indentLevel--;
                    GUILayout.EndVertical();                    
                }

                EditorGUILayout.Space();
            }
        }

//----------------------------------------------------------------------------------------------------------------------
        
        private static void ApplyFrameRate(IEnumerable<AnimationClip> clips, float frameRate) {
            foreach (AnimationClip clip in clips) {
                Undo.RegisterCompleteObjectUndo(clip, "ApplyFrameRate");
                clip.frameRate = frameRate;

                //Debug.Log("Applied frame rate to " + AssetDatabase.GetAssetPath(clip));
            }
            // repaint animation window
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        }

//----------------------------------------------------------------------------------------------------------------------

        private static void ApplyTimeScale(IEnumerable<AnimationClip> clips, float timeScale, float timeOffset) {
            foreach (AnimationClip clip in clips) {
                List<AnimationCurve>     curves   = new List<AnimationCurve>();
                List<EditorCurveBinding> bindings = new List<EditorCurveBinding>();
                AnimationEvent[]         events   = AnimationUtility.GetAnimationEvents(clip);

                // gather curves
                bindings.AddRange(AnimationUtility.GetCurveBindings(clip));
                bindings.AddRange(AnimationUtility.GetObjectReferenceCurveBindings(clip));
                foreach (var b in bindings)
                    curves.Add(AnimationUtility.GetEditorCurve(clip, b));

                int eventCount = events.Length;
                int curveCount = curves.Count;

                // transform keys/events
                foreach (AnimationCurve curve in curves)
                {
                    Keyframe[] keys = curve.keys;
                    int keyCount = keys.Length;
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

                //Debug.Log("Applied time scale to " + AssetDatabase.GetAssetPath(clip));
            }

            // repaint animation window
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        }

//----------------------------------------------------------------------------------------------------------------------        
        private static void ApplyDropKeyframes(IEnumerable<AnimationClip> clips, int step) {
            Assert.IsTrue(step > 1);

            foreach (AnimationClip clip in clips)
            {
                List<AnimationCurve>     curves   = new List<AnimationCurve>();
                List<EditorCurveBinding> bindings = new List<EditorCurveBinding>();

                // gather curves
                bindings.AddRange(AnimationUtility.GetCurveBindings(clip));
                bindings.AddRange(AnimationUtility.GetObjectReferenceCurveBindings(clip));
                foreach (EditorCurveBinding b in bindings)
                    curves.Add(AnimationUtility.GetEditorCurve(clip, b));

                int curveCount = curves.Count;

                // transform keys/events
                foreach (var curve in curves)
                {
                    Keyframe[]     keys     = curve.keys;
                    int            keyCount = keys.Length;
                    List<Keyframe> newKeys  = new List<Keyframe>();
                    for (int ki = 0; ki < keyCount; ki += step)
                        newKeys.Add(keys[ki]);
                    curve.keys = newKeys.ToArray();
                }
 
                // apply changes to clip
                Undo.RegisterCompleteObjectUndo(clip, "ApplyDropKeyframes");
                for (int ci = 0; ci < curveCount; ++ci)
                    Misc.SetCurve(clip, bindings[ci], curves[ci]);

                //Debug.Log("Applied drop keyframes to " + AssetDatabase.GetAssetPath(clip));
            }

            // repaint animation window
            UnityEditorInternal.InternalEditorUtility.RepaintAllViews();
        }
        
//----------------------------------------------------------------------------------------------------------------------        

        private static void ApplyKeyframeReduction(IEnumerable<AnimationClip> clips, float threshold, bool eraseFlatCurves)
        {
            foreach (var clip in clips)
            {
                List<AnimationCurve>     curves   = new List<AnimationCurve>();
                List<EditorCurveBinding> bindings = new List<EditorCurveBinding>();

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

                //Debug.Log("Applied keyframe reduction to " + AssetDatabase.GetAssetPath(clip));
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
