using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync {

//[TODO-sin: 2020-9-24: Move to AnimeToolbox
internal static class GameObjectEditorExtensions {


//----------------------------------------------------------------------------------------------------------------------
    
    public static GameObject SaveAsPrefab(this GameObject go, string prefabPath, 
        InteractionMode mode = InteractionMode.AutomatedAction) 
    {
        return PrefabUtility.SaveAsPrefabAssetAndConnect(go, prefabPath, mode);        
    }
    
}

} //end namespace