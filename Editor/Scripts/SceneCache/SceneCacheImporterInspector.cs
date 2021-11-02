
#if UNITY_2020_2_OR_NEWER
using UnityEditor.AssetImporters;
#else
    using UnityEditor.Experimental.AssetImporters;
#endif

using UnityEditor;

namespace Unity.MeshSync.Editor {

[CustomEditor(typeof(SceneCacheImporter))]
public class SceneCacheImporterInspector: ScriptedImporterEditor
{
    public override void OnInspectorGUI() {

        serializedObject.Update();
        EditorGUILayout.PropertyField(serializedObject.FindProperty(SceneCacheImporter.IMPORTER_SETTINGS_PROP));
        serializedObject.ApplyModifiedProperties();
        base.ApplyRevertGUI();
    }

}


} //end namespace
