using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;
using UnityEngine.Assertions;
using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor  {
internal static class MeshSyncInspectorUtility {
    internal static void DrawModelImporterSettingsGUI(Object obj, ModelImporterSettings settings) {
        EditorGUIDrawerUtility.DrawUndoableGUI(obj, "Create Materials",
            () => (bool)EditorGUILayout.Toggle("Create Materials", settings.CreateMaterials),
            (bool createMat) => { settings.CreateMaterials = createMat; });

        DrawModelImporterMaterialSearchMode(obj, settings);
        
        EditorGUIDrawerUtility.DrawUndoableGUI(obj, "Overwrite Exported Materials",
            () => (bool)EditorGUILayout.Toggle("Overwrite Exported Materials", settings.OverwriteExportedMaterials),
            (bool overwriteMat) => { settings.OverwriteExportedMaterials = overwriteMat; });
        
        EditorGUIDrawerUtility.DrawUndoableGUI(obj, "Default Shader",
            () => EditorGUILayout.ObjectField("Default Shader", settings.DefaultShader, typeof(Shader)),
            (Object defaultShader) => { settings.DefaultShader = defaultShader as Shader; });
    }

    internal static void DrawModelImporterMaterialSearchMode(Object obj, ModelImporterSettings settings) {
        using (new EditorGUI.DisabledScope(!settings.CreateMaterials)) {
            EditorGUIDrawerUtility.DrawUndoableGUI(obj, "Search Mode",
                () => {
                    ++EditorGUI.indentLevel;
                    AssetSearchMode ret = (AssetSearchMode)EditorGUILayout.EnumPopup("Search Mode", settings.MaterialSearchMode);
                    --EditorGUI.indentLevel;
                    return ret;
                },
                (AssetSearchMode mode) => { settings.MaterialSearchMode = mode; });
        }
    }

    internal static void DrawModelImporterSettingsGUI(SerializedProperty property) {
        Assert.IsNotNull(property);

        SerializedProperty createMatProp = property.FindPropertyRelative(ModelImporterSettings.CREATE_MATERIALS_PROP);
        EditorGUILayout.PropertyField(createMatProp, new GUIContent("Create Materials"));

        SerializedProperty overwriteExportedMatProp =
            property.FindPropertyRelative(ModelImporterSettings.OVERWRITE_EXPORTED_MATERIALS);
        EditorGUILayout.PropertyField(overwriteExportedMatProp, new GUIContent("Overwrite Exported Materials"));

        using (new EditorGUI.DisabledScope(!createMatProp.boolValue)) {
            ++EditorGUI.indentLevel;
            EditorGUILayout.PropertyField(property.FindPropertyRelative(ModelImporterSettings.MATERIAL_SEARCH_MODE_PROP),
                new GUIContent("Material Search Mode"));
            --EditorGUI.indentLevel;
        }
    }

//----------------------------------------------------------------------------------------------------------------------

    //returns true if there is any change. False otherwise
    internal static bool DrawComponentSyncSettings(Object obj, string prefixLabel,  ComponentSyncSettings syncSettings) {
        EditorGUILayout.BeginHorizontal();

        EditorGUILayout.PrefixLabel(prefixLabel);
        GUILayout.Space(2);
        bool changed = EditorGUIDrawerUtility.DrawUndoableGUI(obj, "Sync: Create",
            () => (bool)GUILayout.Toggle(syncSettings.CanCreate, "Create"),
            (bool val) => { syncSettings.CanCreate = val; });

        using (new EditorGUI.DisabledScope(!syncSettings.CanCreate)) {
            changed |= EditorGUIDrawerUtility.DrawUndoableGUI(obj, "Sync: Update",
                () => (bool)GUILayout.Toggle(syncSettings.CanUpdate, "Update"),
                (bool val) => { syncSettings.CanUpdate = val; });
        }

        EditorGUILayout.EndHorizontal();
        return changed;
    }
}
} //end namespace