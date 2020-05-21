using System.Collections.Generic;
using UnityEditor;
using System.IO;
using Unity.AnimeToolbox.Editor;
using UnityEngine;
using UnityEngine.UIElements;
using Unity.MeshSync;
using UnityEditor.UIElements;

namespace UnityEditor.MeshSync {
	internal class GeneralSettingsTab : IMeshSyncSettingsTab {
		
        public GeneralSettingsTab() {
        }


//----------------------------------------------------------------------------------------------------------------------        
        public void Setup(VisualElement root) {
            root.Add(new Label("General Settings Content"));
           
	        
	        VisualTreeAsset container = UIElementsEditorUtility.LoadVisualTreeAsset(
		        Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "GeneralSettings_Container")
	        );
	        TemplateContainer containerInstance = container.CloneTree();
	        
	        VisualElement playerSettingsContainer = containerInstance.Query<VisualElement>("PlayerSettingsContainer").First();
	        
	        List<string> objectTypes = new List<string> {
		        MeshSyncObjectType.SERVER.ToString(),
		        MeshSyncObjectType.CACHE_PLAYER.ToString(),
	        };	        
	        //
	        // TemplateContainer containerInstance = container.CloneTree();
	        // ScrollView scrollView = containerInstance.Query<ScrollView>().First();
	        
	        //Add the container of this tab to root
	        PopupField<string> playerSettingsPopup = new PopupField<string>(objectTypes, objectTypes[0]);
	        

	        playerSettingsPopup.RegisterValueChangedCallback(OnToggleValueChanged);
	        playerSettingsContainer.Add(playerSettingsPopup);        
	        
	        //Add toggles	           
	        MeshSyncProjectSettings projectSettings = MeshSyncProjectSettings.GetOrCreateSettings();
	        MeshSyncPlayerConfig playerConfig = projectSettings.GetDefaultPlayerConfig(MeshSyncObjectType.SERVER);
	        
	        VisualTreeAsset toggleTemplate = UIElementsEditorUtility.LoadVisualTreeAsset(
		        Path.Combine(MeshSyncEditorConstants.PROJECT_SETTINGS_UIELEMENTS_PATH, "GeneralSettingsToggleTemplate")
	        );
	        TemplateContainer toggleContainer = toggleTemplate.CloneTree();
	        Toggle toggle = toggleContainer.Query<Toggle>().First();
	        toggle.bindingPath = "a";
//	        toggle.Bind(new SerializedObject(playerConfig));
//	        toggle.bind
	        
	        // hello a = new hello();
	        // a.

	        playerSettingsContainer.Add(toggleContainer);

	        
	        root.Add(containerInstance);
        }

		
		private class SliderProgressTestObject : ScriptableObject
		{
			public int exampleValue = 0;
		}
		

		private void OnSomething(VisualElement e) {
			
		}
		private void OnToggleValueChanged(ChangeEvent<string> changeEvt)
		{
			PopupField<string> target = changeEvt.target as PopupField<string>;
			if (null == target) {
				return;
			}
			
			Debug.Log(target.index);
		}		

	}
}
