using UnityEditor;
using System.IO;
using UnityEngine;
using UnityEngine.UIElements;

namespace UnityEditor.MeshSync {
	internal class GeneralSettingsTab : IMeshSyncSettingsTab {
		
        public GeneralSettingsTab() {
        }


//----------------------------------------------------------------------------------------------------------------------        
        public void Setup(VisualElement root) {
            root.Add(new Label("General Settings Content"));
            
        }
        

	}
}
