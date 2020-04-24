using UnityEditor;
using UnityEditorInternal;

using System.IO;
using System.Linq;
using System.Collections.Generic;
using NUnit.Framework;
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
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            foreach (DCCToolInfo dccToolInfo in settings.GetDCCToolInfos()) {
                AddDCCToolSettingsContainer(dccToolInfo, scrollView, dccToolInfoTemplate);                
            }
            
            Button addDCCToolButton = containerInstance.Query<Button>("AddDCCToolButton").First();
            addDCCToolButton.clickable.clicked += OnAddDCCToolButtonClicked;

            
            //Add the container of this tab to root
            root.Add(containerInstance);
        }

//----------------------------------------------------------------------------------------------------------------------        

        void AddDCCToolSettingsContainer(DCCToolInfo info, VisualElement top, VisualTreeAsset dccToolInfoTemplate) {
            TemplateContainer container = dccToolInfoTemplate.CloneTree();
            container.Query<Label>("DCCToolName").First().text = info.Type.ToString() + " " + info.Version;
            container.Query<Label>("DCCToolPath").First().text = "Path to " + info.Type.ToString();

            if (string.IsNullOrEmpty(info.PluginVersion)) {
                container.Query<Label>("DCCToolStatus").First().text = "Plugin not installed";
            }
            
            top.Add(container);
        }
        
//----------------------------------------------------------------------------------------------------------------------        
        
        static void OnAddDCCToolButtonClicked() {
            //[TODO-sin: 2020-4-24] Show window to add 
            MeshSyncProjectSettings settings = MeshSyncProjectSettings.GetOrCreateSettings();
            settings.AddDCCToolInfo(DCCToolType.AUTODESK_MAYA, "2020");
            
            
            //Set dirty

            
            Debug.Log("Adding DCC Tool");
        }


    }
    
} //end namespace
