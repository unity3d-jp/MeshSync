
namespace Unity.MeshSync.Editor {

using UnityEditor;
using UnityEditor.Experimental.AssetImporters;

[CustomEditor(typeof(SceneCacheImporter))]
public class SceneCacheImporterInspector: ScriptedImporterEditor
{
    public override void OnInspectorGUI() {
        base.ApplyRevertGUI();
    }
}


} //end namespace
