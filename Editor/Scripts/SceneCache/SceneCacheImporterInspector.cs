namespace Unity.MeshSync.Editor {

using UnityEditor;
using UnityEditor.Experimental.AssetImporters;

[CustomEditor(typeof(SceneCacheImporter))]
public class SceneCacheImporterInspector: ScriptedImporterEditor
{
    public override void OnInspectorGUI() {

        serializedObject.Update();
        EditorGUILayout.PropertyField(serializedObject.FindProperty(MeshSyncEditorConstants.SCENE_CACHE_IMPORTER_SETTINGS_PROP));
        serializedObject.ApplyModifiedProperties();
        base.ApplyRevertGUI();
    }

}


} //end namespace
