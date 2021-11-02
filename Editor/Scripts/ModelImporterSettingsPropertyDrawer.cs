using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {

[CustomPropertyDrawer(typeof(ModelImporterSettings))]
public class ModelImporterSettingsPropertyDrawer : PropertyDrawer {
    public override void OnGUI(Rect position, SerializedProperty property, GUIContent label) {
        
        EditorGUI.BeginProperty(position, label, property);
        
        EditorGUILayout.PropertyField(property.FindPropertyRelative("CreateMaterials"), new GUIContent("Create Materials"));
        
        ++EditorGUI.indentLevel;        
        EditorGUILayout.PropertyField(property.FindPropertyRelative("MaterialSearchMode"), new GUIContent("Material Search Mode"));
        --EditorGUI.indentLevel;

        EditorGUI.EndProperty();
    }    
}
}