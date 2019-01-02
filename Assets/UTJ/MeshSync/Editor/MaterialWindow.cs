using UnityEngine;
using UnityEditor;
using UTJ.MeshSync;

namespace UTJ.MeshSyncEditor
{
    class MaterialWindow : EditorWindow
    {
        public static void Open(MeshSyncServer server)
        {
            var window = (MaterialWindow)EditorWindow.GetWindow(typeof(MaterialWindow));
            window.titleContent = new GUIContent("Material List");
            window.m_server = server;
            window.Show();
        }


        public MeshSyncServer m_server;


        void OnGUI()
        {
            if(m_server == null)
            {
                return;
            }

            MeshSyncServerEditor.DrawMaterialList(m_server);
        }
    }
}
