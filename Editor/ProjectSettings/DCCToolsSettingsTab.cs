using UnityEditor;
using UnityEditorInternal;

using System.IO;
using System.Linq;
using System.Collections.Generic;
using Unity.AnimeToolbox;
using UnityEngine;
using UnityEngine.UIElements;


namespace UnityEditor.MeshSync {
	internal class DCCToolsSettingsTab : IMeshSyncSettingsTab{


        internal class Styles {
            public static readonly GUIContent defaultExecutionOrder = EditorGUIUtility.TrTextContent("Default Execution Order");
            public static readonly GUIContent executionOrderLabel = EditorGUIUtility.TrTextContent("AssetPostprocessor Graph Execution Order");
        }
        
        

        public DCCToolsSettingsTab() {
            
            m_dccToolInfoTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
                Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "DCCToolInfoTemplate")
            );
            
        }

//----------------------------------------------------------------------------------------------------------------------        
        public void Setup(VisualElement root) {
            
            root.Add(m_dccToolInfoTemplate.CloneTree());
            root.Add(m_dccToolInfoTemplate.CloneTree());
            root.Add(m_dccToolInfoTemplate.CloneTree());
            root.Add(m_dccToolInfoTemplate.CloneTree());
            root.Add(m_dccToolInfoTemplate.CloneTree());
            
        }
        
//----------------------------------------------------------------------------------------------------------------------        

        private readonly VisualTreeAsset m_dccToolInfoTemplate = null;

    }
}
