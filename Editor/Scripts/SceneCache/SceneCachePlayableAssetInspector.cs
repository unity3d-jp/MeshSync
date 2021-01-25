using Unity.AnimeToolbox;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor {

[CustomEditor(typeof(SceneCachePlayableAsset))]
internal class SceneCachePlayableAssetInspector : UnityEditor.Editor {

    void OnEnable() {
        m_scPlayableAsset = target as SceneCachePlayableAsset;
    }
    
    void OnDisable() {
        m_scPlayableAsset = null;
    }

//----------------------------------------------------------------------------------------------------------------------
    public override void OnInspectorGUI() {
        if (m_scPlayableAsset.IsNullRef())
            return;
        
        SerializedObject so = serializedObject;

        EditorGUILayout.PropertyField(so.FindProperty("m_sceneCachePlayerRef"), SceneCachePlayerRef);
        
        so.ApplyModifiedProperties();
       
    }


//----------------------------------------------------------------------------------------------------------------------

    
    public static readonly GUIContent SceneCachePlayerRef = EditorGUIUtility.TrTextContent("SceneCachePlayerRef");
    
    private SceneCachePlayableAsset m_scPlayableAsset;

}
} //end namespace