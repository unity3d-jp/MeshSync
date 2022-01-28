using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor
{
class MaterialWindow : EditorWindow
{
    public static void Open(BaseMeshSync server)
    {
        MaterialWindow window = (MaterialWindow)EditorWindow.GetWindow(typeof(MaterialWindow));
        window.titleContent = new GUIContent("Material List");
        window.m_meshSyncComponent = server;
        window.Show();
    }

//----------------------------------------------------------------------------------------------------------------------    

    void OnGUI()
    {
        if(m_meshSyncComponent == null)
        {
            return;
        }

        Rect pos = position;
        m_scrollPos = EditorGUILayout.BeginScrollView(m_scrollPos, GUILayout.Width(pos.width), GUILayout.Height(pos.height));
        BaseMeshSyncInspector.DrawSimpleMaterialList(m_meshSyncComponent);
        EditorGUILayout.EndScrollView();
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    private BaseMeshSync m_meshSyncComponent;
    private Vector2      m_scrollPos;
    
}

} //end namespace
