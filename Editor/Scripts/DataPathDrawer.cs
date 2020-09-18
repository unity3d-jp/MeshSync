using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {

[CustomPropertyDrawer(typeof(DataPath))]
class DataPathDrawer : PropertyDrawer {
    
    public override void OnGUI(Rect position, SerializedProperty property, GUIContent label) {
        bool ro = property.FindPropertyRelative("m_readOnly").boolValue;
        SerializedProperty pShowRoot = property.FindPropertyRelative("m_showRootSelector");
        SerializedProperty pRoot = property.FindPropertyRelative("m_root");
        SerializedProperty pLeaf = property.FindPropertyRelative("m_leaf");
        bool isDirectory = property.FindPropertyRelative("m_isDirectory").boolValue;
        if (ro)
            EditorGUI.BeginDisabledGroup(true);

        EditorGUI.BeginProperty(position, label, property);
        position = EditorGUI.PrefixLabel(position, GUIUtility.GetControlID(FocusType.Passive), label);

        int indent = EditorGUI.indentLevel;
        EditorGUI.indentLevel = 0;

        bool showRoot = pShowRoot.boolValue && pRoot.intValue != (int)DataPath.Root.Current;
        float buttonWidth = 22;
        float rootWidth = showRoot ? 70 : 0;
        float rootMargin = showRoot ? 5 : 0;
        float leafWidth = position.width - rootWidth - rootMargin - buttonWidth;
        Rect rootRect = new Rect(position.x, position.y, rootWidth, position.height);
        Rect leafRect = new Rect(position.x + rootWidth + rootMargin, position.y, leafWidth, position.height);
        Rect buttonRect = new Rect(position.x + rootWidth + rootMargin + leafWidth, position.y, buttonWidth, position.height);

        if (showRoot)
            EditorGUI.PropertyField(rootRect, pRoot, GUIContent.none);
        
        EditorGUI.PropertyField(leafRect, pLeaf, GUIContent.none);
        if (GUI.Button(buttonRect, "...")) {
            DataPath tmp = new DataPath((DataPath.Root)pRoot.intValue, pLeaf.stringValue);
            string path = isDirectory ?
                EditorUtility.OpenFolderPanel("Select Directory", tmp.GetFullPath(), "") :
                EditorUtility.OpenFilePanel("Select File", tmp.GetFullPath(), "");
            if (path.Length > 0) {
                DataPath newPath = new DataPath(path);
                pRoot.intValue = (int)newPath.GetRoot();
                pLeaf.stringValue = newPath.GetLeaf();
            }
        }

        EditorGUI.indentLevel = indent;
        EditorGUI.EndProperty();

        if (ro)
            EditorGUI.EndDisabledGroup();
    }
}

} //end namespace
