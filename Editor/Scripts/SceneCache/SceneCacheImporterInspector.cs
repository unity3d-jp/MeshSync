
using Unity.FilmInternalUtilities.Editor;
using UnityEditor.UIElements;
using UnityEngine;

namespace Unity.MeshSync.Editor {

using UnityEditor;
using UnityEditor.Experimental.AssetImporters;

[CustomEditor(typeof(SceneCacheImporter))]
public class SceneCacheImporterInspector: ScriptedImporterEditor
{
    public override void OnEnable() {
        base.OnEnable();
        m_scImporter = target as SceneCacheImporter;

    }
    
    public override void OnInspectorGUI() {
        DrawModelImporterSettingsGUI(m_scImporter, m_scImporter.GetModelImporterSettings());
        base.ApplyRevertGUI();
    }

    static void DrawModelImporterSettingsGUI(Object obj, ModelImporterSettings settings) {
        EditorGUIDrawerUtility.DrawUndoableGUI(obj, "SceneCache: Snap",            
            guiFunc: () => (bool)EditorGUILayout.Toggle("Create Materials", settings.CreateMaterials), 
            updateFunc: (bool createMat) => { settings.CreateMaterials = createMat; });
        
    }

//----------------------------------------------------------------------------------------------------------------------
    
    private SceneCacheImporter m_scImporter = null;

}


} //end namespace
