using System;
using System.Collections.Generic;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {
internal abstract class BaseMeshSyncInspector : UnityEditor.Editor {
//----------------------------------------------------------------------------------------------------------------------

    protected static bool DrawAssetSyncSettings(BaseMeshSync t) {
        t.foldSyncSettings = EditorGUILayout.Foldout(t.foldSyncSettings, "Asset Sync Settings", true, GetBoldFoldoutStyle());
        MeshSyncPlayerConfig config = t.GetConfigV();
        if (!t.foldSyncSettings)
            return false;

        bool changed = false;

        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Sync Transform",
            () => EditorGUILayout.Toggle("Update Transform", config.SyncTransform),
            (bool toggle) => { config.SyncTransform = toggle; }
        );

        ComponentSyncSettings syncCameraSettings = config.GetComponentSyncSettings(MeshSyncPlayerConfig.SYNC_CAMERA);
        changed |= MeshSyncInspectorUtility.DrawComponentSyncSettings(t, "Cameras", syncCameraSettings);

        using (new EditorGUI.DisabledScope(!(syncCameraSettings.CanCreate && syncCameraSettings.CanUpdate))) {
            EditorGUI.indentLevel++;

            changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Physical Camera Params",
                () => EditorGUILayout.Toggle("Use Physical Params", config.IsPhysicalCameraParamsUsed()),
                (bool toggle) => { config.UsePhysicalCameraParams(toggle); }
            );

            EditorGUI.indentLevel--;
        }

        changed |= MeshSyncInspectorUtility.DrawComponentSyncSettings(t, "Lights",
            config.GetComponentSyncSettings(MeshSyncPlayerConfig.SYNC_LIGHTS));


        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Sync Meshes",
            () => EditorGUILayout.Toggle("Meshes", config.SyncMeshes),
            (bool toggle) => { config.SyncMeshes = toggle; }
        );

        EditorGUI.indentLevel++;
        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Update Mesh Colliders",
            () => EditorGUILayout.Toggle("Update Mesh Colliders", config.UpdateMeshColliders),
            (bool toggle) => { config.UpdateMeshColliders = toggle; }
        );

#if AT_USE_PROBUILDER
        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Use Pro Builder",
            guiFunc: () => EditorGUILayout.Toggle("Use Pro Builder", t.UseProBuilder),
            updateFunc: (bool toggle) => { t.UseProBuilder = toggle; }
        );
        EditorGUI.indentLevel--;
#endif

        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Sync Visibility",
            () => EditorGUILayout.Toggle("Visibility", config.SyncVisibility),
            (bool toggle) => { config.SyncVisibility = toggle; }
        );

        EditorGUILayout.Space();

        return changed;
    }

    protected static bool DrawImportSettings(BaseMeshSync t) {
        bool                 changed      = false;
        MeshSyncPlayerConfig playerConfig = t.GetConfigV();

        t.foldImportSettings = EditorGUILayout.Foldout(t.foldImportSettings, "Import Settings", true, GetBoldFoldoutStyle());
        if (t.foldImportSettings) {
            MeshSyncInspectorUtility.DrawModelImporterSettingsGUI(t, playerConfig.GetModelImporterSettings());

            changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Animation Interpolation",
                () => EditorGUILayout.Popup(new GUIContent("Animation Interpolation"),
                    playerConfig.AnimationInterpolation, MeshSyncEditorConstants.ANIMATION_INTERPOLATION_ENUMS),
                (int val) => { playerConfig.AnimationInterpolation = val; }
            );


            changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Keyframe Reduction",
                () => EditorGUILayout.Toggle("Keyframe Reduction", playerConfig.KeyframeReduction),
                (bool toggle) => { playerConfig.KeyframeReduction = toggle; }
            );

            if (playerConfig.KeyframeReduction) {
                EditorGUI.indentLevel++;

                changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Threshold",
                    () => EditorGUILayout.FloatField("Threshold", playerConfig.ReductionThreshold),
                    (float val) => { playerConfig.ReductionThreshold = val; }
                );

                changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Erase Flat Curves",
                    () => EditorGUILayout.Toggle("Erase Flat Curves", playerConfig.ReductionEraseFlatCurves),
                    (bool toggle) => { playerConfig.ReductionEraseFlatCurves = toggle; }
                );
                EditorGUI.indentLevel--;
            }

            changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Z-Up Correction",
                () => EditorGUILayout.Popup(new GUIContent("Z-Up Correction"), playerConfig.ZUpCorrection,
                    MeshSyncEditorConstants.Z_UP_CORRECTION_ENUMS),
                (int val) => { playerConfig.ZUpCorrection = val; }
            );

            EditorGUILayout.Space();
        }

        return changed;
    }

    protected static bool DrawMiscSettings(BaseMeshSync t) {
        MeshSyncPlayerConfig playerConfig = t.GetConfigV();

        // Misc
        t.foldMisc = EditorGUILayout.Foldout(t.foldMisc, "Misc", true, GetBoldFoldoutStyle());
        if (!t.foldMisc)
            return false;

        bool changed = false;

        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Sync Material List",
            () => EditorGUILayout.Toggle("Sync Material List", playerConfig.SyncMaterialList),
            (bool toggle) => { playerConfig.SyncMaterialList = toggle; }
        );

        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Progressive Display",
            () => EditorGUILayout.Toggle("Progressive Display", playerConfig.ProgressiveDisplay),
            (bool toggle) => { playerConfig.ProgressiveDisplay = toggle; }
        );


        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Logging",
            () => EditorGUILayout.Toggle("Logging", playerConfig.Logging),
            (bool toggle) => { playerConfig.Logging = toggle; }
        );

        changed |= EditorGUIDrawerUtility.DrawUndoableGUI(t, "MeshSync: Profiling",
            () => EditorGUILayout.Toggle("Profiling", playerConfig.Profiling),
            (bool toggle) => { playerConfig.Profiling = toggle; }
        );

        EditorGUILayout.Space();

        return changed;
    }

//----------------------------------------------------------------------------------------------------------------------
    internal static bool DrawDefaultMaterialList(BaseMeshSync t) {
        GUIStyle styleFold = EditorStyles.foldout;
        styleFold.fontStyle = FontStyle.Bold;
        t.foldMaterialList  = EditorGUILayout.Foldout(t.foldMaterialList, "Materials", true, styleFold);
        if (!t.foldMaterialList)
            return false;

        var playerConfig = t.GetConfigV();

        bool changed = DrawMaterialListElements(t);
        DrawMaterialImportExportButtons(t);
        
        if (GUILayout.Button("Open Material Window", GUILayout.Width(160.0f)))
            MaterialWindow.Open(t);
        EditorGUILayout.Space();

        return changed;
    }

    internal static bool DrawSimpleMaterialList(BaseMeshSync t) {
        GUILayout.Label("Materials", EditorStyles.boldLabel);
        bool changed = DrawMaterialListElements(t);
        DrawMaterialImportExportButtons(t);
        return changed;
    }


    private static void DrawMaterialImportExportButtons(BaseMeshSync t) {
        GUILayout.BeginHorizontal();
        if (GUILayout.Button("Import List", GUILayout.Width(110.0f))) {
            string path = EditorUtility.OpenFilePanel("Import material list", "Assets", "asset");
            t.ImportMaterialList(path);
        }

        if (GUILayout.Button("Export List", GUILayout.Width(110.0f))) {
            string path = EditorUtility.SaveFilePanel("Export material list", "Assets", t.name + "_MaterialList", "asset");
            t.ExportMaterialList(path);
        }

        GUILayout.EndHorizontal();
    }


    //returns true if changed
    private static bool DrawMaterialListElements(BaseMeshSync t) {
        // calculate label width
        float labelWidth = 60; // minimum
        {
            GUIStyle style = GUI.skin.box;
            foreach (MaterialHolder md in t.materialList) {
                Vector2 size = style.CalcSize(new GUIContent(md.name));
                labelWidth = Mathf.Max(labelWidth, size.x);
            }

            // 100: margin for color and material field
            labelWidth = Mathf.Min(labelWidth, EditorGUIUtility.currentViewWidth - 100);
        }

        bool changed = false;
        foreach (MaterialHolder matHolder in t.materialList) {
            Rect rect = EditorGUILayout.BeginHorizontal();
            EditorGUI.DrawRect(new Rect(rect.x, rect.y, 16, 16), matHolder.color);
            EditorGUILayout.LabelField("", GUILayout.Width(16));
            EditorGUILayout.LabelField(matHolder.name, GUILayout.Width(labelWidth));
            {
                Material destMat = EditorGUILayout.ObjectField(matHolder.material, typeof(Material), true) as Material;
                if (destMat != matHolder.material) {
                    t.AssignMaterial(matHolder, destMat);
                    changed = true;
                }
            }
            EditorGUILayout.EndHorizontal();
        }

        return changed;
    }

//----------------------------------------------------------------------------------------------------------------------        

    protected static void DrawExportAssets(BaseMeshSync t) {
        GUIStyle style = EditorStyles.foldout;
        style.fontStyle    = FontStyle.Bold;
        t.foldExportAssets = EditorGUILayout.Foldout(t.foldExportAssets, "Export Assets", true, style);
        if (t.foldExportAssets) {
            GUILayout.BeginHorizontal();
            if (GUILayout.Button("Export Meshes", GUILayout.Width(160.0f)))
                t.ExportMeshes();

            if (GUILayout.Button("Export Materials", GUILayout.Width(160.0f)))
                t.ExportMaterials();
            GUILayout.EndHorizontal();
        }

        EditorGUILayout.Space();
    }

    protected static void DrawPluginVersion() {
        EditorGUILayout.LabelField("Plugin Version: " + Lib.GetPluginVersion());
    }

    protected static GUIStyle GetBoldFoldoutStyle() {
        GUIStyle boldFoldoutStyle = EditorStyles.foldout;
        boldFoldoutStyle.fontStyle = FontStyle.Bold;
        return boldFoldoutStyle;
    }
}
} // end namespace