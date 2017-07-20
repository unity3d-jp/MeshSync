using System;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

namespace UTJ.MeshSync
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
