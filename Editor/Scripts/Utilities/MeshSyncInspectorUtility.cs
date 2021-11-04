using Unity.FilmInternalUtilities.Editor;
using UnityEditor;
using UnityEngine;
using UnityEngine.Assertions;
using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor  {

internal static class MeshSyncInspectorUtility {

    internal static void DrawModelImporterSettingsGUI(Object obj, ModelImporterSettings settings) {
        EditorGUIDrawerUtility.DrawUndoableGUI(obj, "Create Materials",            
            guiFunc: () => (bool)EditorGUILayout.Toggle("Create Materials", settings.CreateMaterials), 
            updateFunc: (bool createMat) => { settings.CreateMaterials = createMat; });


        DrawModelImporterMaterialSearchMode(obj, settings);
    }

    internal static void DrawModelImporterMaterialSearchMode(Object obj, ModelImporterSettings settings) {
        using (new EditorGUI.DisabledScope(!settings.CreateMaterials)) {
            
            EditorGUIDrawerUtility.DrawUndoableGUI(obj, "Search Mode",            
                guiFunc: () => {
                    ++EditorGUI.indentLevel;
                    AssetSearchMode ret = (AssetSearchMode)EditorGUILayout.EnumPopup("Search Mode", settings.MaterialSearchMode);                    
                    --EditorGUI.indentLevel;
                    return ret;
                }, 
                updateFunc: (AssetSearchMode mode) => { settings.MaterialSearchMode = mode; });
        }
    }

    internal static void DrawModelImporterSettingsGUI(SerializedProperty property) {
        Assert.IsNotNull(property);

        SerializedProperty createMatProp = property.FindPropertyRelative(ModelImporterSettings.CREATE_MATERIALS_PROP); 
        EditorGUILayout.PropertyField(createMatProp, new GUIContent("Create Materials"));
        
        using (new EditorGUI.DisabledScope(!createMatProp.boolValue)) {
            ++EditorGUI.indentLevel;
            EditorGUILayout.PropertyField(property.FindPropertyRelative(ModelImporterSettings.MATERIAL_SEARCH_MODE_PROP),
                new GUIContent("Material Search Mode"));
            --EditorGUI.indentLevel;
        }
    }
    
}

} //end namespace