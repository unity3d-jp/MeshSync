namespace Unity.MeshSync.Editor {

using UnityEditor;
using UnityEditor.Experimental.AssetImporters;

[CustomEditor(typeof(SceneCacheImporter))]
public class SceneCacheImporterInspector: ScriptedImporterEditor
{
    public override void OnInspectorGUI() {

        serializedObject.Update();
        EditorGUILayout.PropertyField(serializedObject.FindProperty("m_importerSettings"));
        serializedObject.ApplyModifiedProperties();
        base.ApplyRevertGUI();
    }

}


} //end namespace
