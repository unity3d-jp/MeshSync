namespace Unity.MeshSync.Editor {

using UnityEditor;
using UnityEditor.Experimental.AssetImporters;

[CustomEditor(typeof(SceneCacheImporter))]
public class SceneCacheImporterInspector: ScriptedImporterEditor
{
    public override void OnInspectorGUI() {

        EditorGUILayout.PropertyField(serializedObject.FindProperty("m_importerSettings"));
        base.ApplyRevertGUI();
    }

}


} //end namespace
