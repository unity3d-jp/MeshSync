using System.Reflection;
using Unity.FilmInternalUtilities.Editor;
using UnityEditor;

using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor  {

internal static class MeshSyncInspectorUtility {

    internal static void DrawModelImporterSettingsGUI(Object obj, ModelImporterSettings settings) {
        EditorGUIDrawerUtility.DrawUndoableGUI(obj, "Create Materials",            
            guiFunc: () => (bool)EditorGUILayout.Toggle("Create Materials", settings.CreateMaterials), 
            updateFunc: (bool createMat) => { settings.CreateMaterials = createMat; });

        
        using (new EditorGUI.DisabledScope(!settings.CreateMaterials)) {
            
            EditorGUIDrawerUtility.DrawUndoableGUI(obj, "Search Mode",            
                guiFunc: () => {
                    ++EditorGUI.indentLevel;
                    AssetSearchMode ret = (AssetSearchMode)EditorGUILayout.EnumPopup("Search Mode", settings.MaterialSearchMode);                    
                    --EditorGUI.indentLevel;
                    return ret;
                }, 
                updateFunc: (AssetSearchMode mode) => { settings.MaterialSearchMode = mode; });
        }
        
        
        
    }

}

} //end namespace