using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync.Editor
{
    // Partial class for now to make merging code easier later.
    partial class MeshSyncServerInspector
    {
        void DrawSliders(MeshSyncServer server)
        {
            var style = EditorStyles.foldout;
            style.fontStyle = FontStyle.Bold;
            server.foldBlenderSettings = EditorGUILayout.Foldout(server.foldBlenderSettings, "Blender properties", true, style);
            if (!server.foldBlenderSettings)
            {
                return;
            }

            var properties = server.propertyInfos;

            for (int i = 0; i < properties.Count; i++)
            {
                var prop = properties[i];
                EditorGUI.BeginChangeCheck();

                object newValue = null;
                switch (prop.type)
                {
                    case PropertyInfoData.Type.Int:
                        newValue = (int)EditorGUILayout.Slider(prop.name, prop.ValueInt, prop.min, prop.max);
                        break;

                    case PropertyInfoData.Type.Float:
                        newValue = EditorGUILayout.Slider(prop.name, prop.ValueFloat, prop.min, prop.max);
                        break;
                }

                if (EditorGUI.EndChangeCheck())
                {
                    prop.NewValue = newValue;
                }
            }
        }
    }
}
