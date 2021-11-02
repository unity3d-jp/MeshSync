using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor  {

internal static class MeshSyncInspectorUtility {
    internal static void DrawModelImporterSettingsGUI(SerializedProperty property) {

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