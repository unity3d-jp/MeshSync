using System.IO;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;


namespace Unity.AnimeToolbox {

//[TODO-sin: 2020-4-23] : Move this to AnimeToolbox
internal class UIElementsEditorUtility {
    
    /// <summary>
    /// Load a UXML file
    /// </summary>
    /// <param name="pathWithoutExt">Path to the UXML file without the extension</param>
    /// <param name="ext">The extension of the UXML file. Assumed to be ".uxml" </param>
    /// <returns></returns>
    internal static VisualTreeAsset LoadVisualTreeAsset(string pathWithoutExt, string ext = ".uxml") {
        string path = pathWithoutExt + ext;
        VisualTreeAsset asset = AssetDatabase.LoadAssetAtPath<VisualTreeAsset>(path);
        if (null == asset) {
            Debug.LogError("[AnimeToolbox] Can't load VisualTreeAsset: " + path);
            return null;
        }
        return asset;
    }    

    /// <summary>
    /// Load UIElement style file and adds it to StyleSheetSet
    /// </summary>
    /// <param name="set">StyleSheetSet to which the new StyleSheet will be added</param>
    /// <param name="pathWithoutExt">Path to the file without the extension</param>
    /// <param name="ext">The extension of the file. Assumed to be ".uss" </param>
    /// <returns></returns>
    internal static void LoadAndAddStyle(VisualElementStyleSheetSet set, string pathWithoutExt, string ext = ".uss") {
        string path = pathWithoutExt + ext;
        StyleSheet asset = AssetDatabase.LoadAssetAtPath<StyleSheet>(path);
        if (null == asset) {
            Debug.LogError("[AnimeToolbox] Can't load style: " + path);
            return;
        }
        set.Add(asset);
    }    
}


} //namespace Unity.AnimeToolbox
