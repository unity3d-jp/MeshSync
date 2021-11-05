using UnityEngine.UIElements;
using Constants = Unity.MeshSync.Editor.MeshSyncEditorConstants;

namespace Unity.MeshSync.Editor {

internal class ComponentSyncSettingsUI {
    internal ComponentSyncSettingsUI(int syncIndex ) {
        SyncIndex = syncIndex;
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    
    internal readonly int SyncIndex = 0;
    
    internal Toggle CanCreateToggle;
    internal Toggle CanUpdateToggle;
}

} //end namespace 
