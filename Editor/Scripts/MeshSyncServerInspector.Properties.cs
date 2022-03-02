using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync.Editor
{
    // Partial class for now to make merging code easier later.
    partial class MeshSyncServerInspector
    {
        void DrawSliders(BaseMeshSync player)
        {
            var style = EditorStyles.foldout;
            style.fontStyle = FontStyle.Bold;
            player.foldBlenderSettings = EditorGUILayout.Foldout(player.foldBlenderSettings, "Blender properties", true, style);
            if (!player.foldBlenderSettings)
            {
                return;
            }

            var properties = player.propertyInfos;

            for (int i = 0; i < properties.Count; i++)
            {
                PropertyInfoData prop = properties[i];
                EditorGUI.BeginChangeCheck();

                switch (prop.type)
                {
                    case PropertyInfoData.Type.Int:
                        prop.newValue = (int)EditorGUILayout.Slider(prop.name, prop.ValueInt, prop.min, prop.max);
                        break;

                    case PropertyInfoData.Type.Float:
                        prop.newValue = EditorGUILayout.Slider(prop.name, prop.ValueFloat, prop.min, prop.max);
                        break;
                }

                if (EditorGUI.EndChangeCheck())
                {
                    properties[i] = prop;

                    // TODO: Send modifiers back to blender here.
                }
            }
        }
    }
}
