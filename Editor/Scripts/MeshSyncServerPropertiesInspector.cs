using UnityEditor;

namespace Unity.MeshSync.Editor {
[CustomEditor(typeof(MeshSyncServerLiveEditProperties))]
internal class MeshSyncServerPropertiesInspector : UnityEditor.Editor {
    public override void OnInspectorGUI() {
        MeshSyncServerLiveEditProperties propertiesHolder = (MeshSyncServerLiveEditProperties)target;

        MeshSyncServerInspectorUtils.DrawSliderForProperties(propertiesHolder.propertyInfos, false);
    }
}
}