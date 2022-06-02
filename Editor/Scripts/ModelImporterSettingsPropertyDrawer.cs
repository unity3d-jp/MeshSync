using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {

[CustomPropertyDrawer(typeof(ModelImporterSettings))]
internal class ModelImporterSettingsPropertyDrawer : PropertyDrawer {
    public override void OnGUI(Rect position, SerializedProperty property, GUIContent label) {
        
        EditorGUI.BeginProperty(position, label, property);        
        MeshSyncInspectorUtility.DrawModelImporterSettingsGUI(property);
        EditorGUI.EndProperty();
    }    
}
}