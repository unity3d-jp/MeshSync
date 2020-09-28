using UnityEngine;

#if UNITY_EDITOR
using UnityEditor;
#endif


namespace Unity.MeshSync {

//[TODO-sin: 2020-9-24: Move to AnimeToolbox
internal static class GameObjectExtensions {

    public static void DestroyChildrenImmediate(this GameObject go) {
        Transform t = go.transform;
        DestroyChildrenImmediate(t);
    }

    public static void DestroyChildrenImmediate(this Transform t) {                
        int childCount = t.childCount;        
        for (int i = childCount - 1; i >= 0; --i) {
            Object.DestroyImmediate(t.GetChild(i).gameObject, true);
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


    /// <summary>
    /// Checks if a gameObject is a prefab. Instanced prefabs also return true.
    /// </summary>
    /// <param name="gameObject">the gameObject to be checked</param>
    /// <returns>Returns true if the gameObject is a prefab, false otherwise.</returns>
    public static bool IsPrefab(this GameObject gameObject) {
        PrefabAssetType prefabAssetType = PrefabUtility.GetPrefabAssetType(gameObject);
        return (prefabAssetType != PrefabAssetType.NotAPrefab);
    }
    
#endif    
    
    
    
}

} //end namespace