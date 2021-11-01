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
        MeshSyncInspectorUtility.DrawModelImporterSettingsGUI(m_scImporter, m_scImporter.GetModelImporterSettings());
        base.ApplyRevertGUI();
    }

//----------------------------------------------------------------------------------------------------------------------
    
    private SceneCacheImporter m_scImporter = null;

}


} //end namespace
