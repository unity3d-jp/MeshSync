using System;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

namespace UTJ
{
    class MQMaterialWindow : EditorWindow
    {
        public static void Open(MeshSyncServer server)
        {
            var window = (MQMaterialWindow)EditorWindow.GetWindow(typeof(MQMaterialWindow));
            window.titleContent = new GUIContent("MQ Materials");
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
