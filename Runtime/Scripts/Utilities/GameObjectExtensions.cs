using UnityEngine;

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

   
    
}

} //end namespace