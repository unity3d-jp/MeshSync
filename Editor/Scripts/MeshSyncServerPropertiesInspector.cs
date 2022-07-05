using UnityEditor;

namespace Unity.MeshSync.Editor
{
    [CustomEditor(typeof(MeshSyncServerLiveEditProperties))]
    internal class MeshSyncServerPropertiesInspector : UnityEditor.Editor
    {
        public override void OnInspectorGUI()
        {
            var propertiesHolder = (MeshSyncServerLiveEditProperties)target;

            MeshSyncServerInspectorUtils.DrawSliderForProperties(propertiesHolder.propertyInfos, editableStrings: false);
        }
    }
}
