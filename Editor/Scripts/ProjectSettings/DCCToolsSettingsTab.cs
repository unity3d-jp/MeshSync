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
            
            
        }

//----------------------------------------------------------------------------------------------------------------------        
        public void Setup(VisualElement root) {

            VisualTreeAsset container = UIElementsEditorUtility.LoadVisualTreeAsset(
                Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "DCCToolsSettings_Container")
            );
            
            VisualTreeAsset dccToolInfoTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
                Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "DCCToolInfoTemplate")
            );

            TemplateContainer containerInstance = container.CloneTree();
            ScrollView scrollView = containerInstance.Query<ScrollView>().First();

            //[TODO-sin: 2020-4-24] Auto detect installed DCC tools + check MeshSync status
            scrollView.Add(dccToolInfoTemplate.CloneTree());
            scrollView.Add(dccToolInfoTemplate.CloneTree());
            scrollView.Add(dccToolInfoTemplate.CloneTree());
            scrollView.Add(dccToolInfoTemplate.CloneTree());
            scrollView.Add(dccToolInfoTemplate.CloneTree());
            
            Button addDCCToolButton = containerInstance.Query<Button>("AddDCCToolButton").First();
            addDCCToolButton.RegisterCallback<MouseDownEvent>(OnAddDCCToolButtonMouseDown);

            
            //Add the container of this tab to root
            root.Add(containerInstance);
        }
        
//----------------------------------------------------------------------------------------------------------------------        
        
        static void OnAddDCCToolButtonMouseDown(MouseEventBase<MouseDownEvent> evt) {
            Debug.Log("Adding DCC Tool");
        }
        


    }
    
} //end namespace
