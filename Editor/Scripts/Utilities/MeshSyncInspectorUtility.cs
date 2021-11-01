using Unity.FilmInternalUtilities.Editor;
using UnityEditor;

using Object = UnityEngine.Object;

namespace Unity.MeshSync.Editor  {

internal static class MeshSyncInspectorUtility {

    static void DrawModelImporterSettingsGUI(Object obj, ModelImporterSettings settings) {
        EditorGUIDrawerUtility.DrawUndoableGUI(obj, "SceneCache: Snap",            
            guiFunc: () => (bool)EditorGUILayout.Toggle("Create Materials", settings.CreateMaterials), 
            updateFunc: (bool createMat) => { settings.CreateMaterials = createMat; });
        
    }

}

} //end namespace