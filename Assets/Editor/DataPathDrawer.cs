using UnityEngine;
using UnityEditor;
using UTJ.MeshSync;

namespace UTJ.MeshSyncEditor
{
    [CustomPropertyDrawer(typeof(DataPath))]
    class DataPathDrawer : PropertyDrawer
    {
        public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
        {
            bool ro = property.FindPropertyRelative("m_readOnly").boolValue;
            bool showRoot = property.FindPropertyRelative("m_showRootSelector").boolValue;
            if (ro)
                EditorGUI.BeginDisabledGroup(true);

            EditorGUI.BeginProperty(position, label, property);
            position = EditorGUI.PrefixLabel(position, GUIUtility.GetControlID(FocusType.Passive), label);

            var indent = EditorGUI.indentLevel;
            EditorGUI.indentLevel = 0;

            float buttonWidth = 22;
            float rootWidth = showRoot ? 70 : 0;
            float rootMargin = showRoot ? 5 : 0;
            float leafWidth = position.width - rootWidth - rootMargin - buttonWidth;
            var rootRect = new Rect(position.x, position.y, rootWidth, position.height);
            var leafRect = new Rect(position.x + rootWidth + rootMargin, position.y, leafWidth, position.height);
            var buttonRect = new Rect(position.x + rootWidth + rootMargin + leafWidth, position.y, buttonWidth, position.height);

            var pRoot = property.FindPropertyRelative("m_root");
            var pLeaf = property.FindPropertyRelative("m_leaf");
            if (showRoot)
                EditorGUI.PropertyField(rootRect, pRoot, GUIContent.none);
            EditorGUI.PropertyField(leafRect, pLeaf, GUIContent.none);
            if (GUI.Button(buttonRect, "..."))
            {
                var tmp = new DataPath((DataPath.Root)pRoot.intValue, pLeaf.stringValue);
                var path = EditorUtility.OpenFolderPanel("Select Directory", tmp.fullPath, "");
                if (path.Length > 0)
                {
                    var newPath = new DataPath(path);
                    pRoot.intValue = (int)newPath.root;
                    pLeaf.stringValue = newPath.leaf;
                }
            }

            EditorGUI.indentLevel = indent;
            EditorGUI.EndProperty();

            if (ro)
                EditorGUI.EndDisabledGroup();
        }
    }
}
