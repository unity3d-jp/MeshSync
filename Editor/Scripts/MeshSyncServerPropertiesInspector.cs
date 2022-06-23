using UnityEditor;

namespace Unity.MeshSync.Editor
{
    [CustomEditor(typeof(MeshSyncServerProperties))]
    internal class MeshSyncServerPropertiesInspector : UnityEditor.Editor
    {
        public override void OnInspectorGUI()
        {
            var propertiesHolder = (MeshSyncServerProperties)target;

            MeshSyncServerInspectorUtils.DrawSliderForProperties(propertiesHolder.propertyInfos, editableStrings: false);
        }
    }
}
