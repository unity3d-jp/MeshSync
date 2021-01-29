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
        EditorGUILayout.PropertyField(so.FindProperty("m_sceneCachePlayerRef"), SCENE_CACHE_PLAYER);
                
        {
            // Curve Operations
            GUILayout.BeginVertical("Box");
            EditorGUILayout.LabelField("Curves", EditorStyles.boldLabel);

            const float BUTTON_X     = 30;
            const float BUTTON_WIDTH = 160f;
            if (DrawGUIButton(BUTTON_X, BUTTON_WIDTH,"To Linear")) {
                SceneCacheClipData clipData = m_scPlayableAsset.GetBoundClipData();
                clipData.SetCurveToLinear();
            }
            
            if (DrawGUIButton(BUTTON_X, BUTTON_WIDTH,"Apply Original")) {
                SceneCacheClipData clipData = m_scPlayableAsset.GetBoundClipData();
                clipData.ApplyOriginalSceneCacheCurve();                
            }
            
            GUILayout.EndVertical();                    
        }
        
        so.ApplyModifiedProperties();       
    }

//----------------------------------------------------------------------------------------------------------------------
    private bool DrawGUIButton(float leftX, float width, string buttonText) {
        Rect rect = GUILayoutUtility.GetRect(new GUIContent("Button"),GUI.skin.button, GUILayout.Width(width));
        rect.x += leftX;
        return (GUI.Button(rect, buttonText, GUI.skin.button));        
    }

//----------------------------------------------------------------------------------------------------------------------


    private static readonly GUIContent SCENE_CACHE_PLAYER = EditorGUIUtility.TrTextContent("Scene Cache Player");
    
    private SceneCachePlayableAsset m_scPlayableAsset;

}
} //end namespace