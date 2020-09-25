using UnityEngine;

#if UNITY_EDITOR
using UnityEditor;
#endif


namespace Unity.MeshSync {

//[TODO-sin: 2020-9-24: Move to AnimeToolbox
internal static class GameObjectExtensions {

    public static void DestroyChildrenImmediate(this GameObject go) {        
        foreach (Transform child in go.transform) {
            Object.DestroyImmediate(child.gameObject);
        }        
    }

    public static void DestroyChildrenImmediate(this Transform t) {        
        foreach (Transform child in t) {
            Object.DestroyImmediate(child.gameObject);
        }        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    /// <summary>
    /// Returns the component of Type type. If one doesn't already exist on the GameObject it will be added.
    /// </summary>
    /// <typeparam name="T">The type of Component to return.</typeparam>
    /// <param name="gameObject">The GameObject this Component is attached to.</param>
    /// <returns>Component</returns>
    public static T GetOrAddComponent<T>(this GameObject gameObject) where T : Component {
        return gameObject.GetComponent<T>() ?? gameObject.AddComponent<T>();
    }

   
//----------------------------------------------------------------------------------------------------------------------
#if UNITY_EDITOR
    
    public static GameObject SaveAsPrefab(this GameObject go, string prefabPath, 
        InteractionMode mode = InteractionMode.AutomatedAction) 
    {
        return PrefabUtility.SaveAsPrefabAssetAndConnect(go, prefabPath, mode);        
    }
    
    public static bool IsPrefabInstance(this GameObject gameObject) {
        PrefabInstanceStatus prefabInstanceStatus = PrefabUtility.GetPrefabInstanceStatus(gameObject);
        return (prefabInstanceStatus != PrefabInstanceStatus.NotAPrefab);

    }
    
#endif    
    
    
    
}

} //end namespace