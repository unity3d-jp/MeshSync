using UnityEditor;
using System.IO;
using UnityEngine;
using UnityEngine.UIElements;

namespace UnityEditor.MeshSync {
	internal class GeneralSettingsTab : IMeshSyncSettingsTab {

        internal class Styles
        {
            public static readonly GUIContent defaultAssetBundleGraph = EditorGUIUtility.TrTextContent("Default AssetBundle Graph");
            public static readonly GUIContent assetBundleBuildMapFile = EditorGUIUtility.TrTextContent("AssetBundle Build Map File");
            public static readonly GUIContent setButton = EditorGUIUtility.TrTextContent("Set");
            public static readonly GUIContent bundleCacheDirectoryLabel = EditorGUIUtility.TrTextContent("Bundle Cache Directory");
            public static readonly GUIContent configDirectoryLabel = EditorGUIUtility.TrTextContent("Config Directory");

            public static readonly GUIContent help_bundleCacheDirectory = EditorGUIUtility.TrTextContent(
                "Bundle Cache Directory is the default place to save AssetBundles when 'Build Asset Bundles' node performs build. This can be set outside of the project with relative path."
            );
            
            public static readonly GUIContent help_defaultAssetGraph = EditorGUIUtility.TrTextContent(
                "Default AssetBundle Graph is the default graph to build AssetBundles for this project. This graph will be automatically called in AssetBundle Browser integration."
            );
            public static readonly GUIContent help_assetBundleBuildMap = EditorGUIUtility.TrTextContent(
                "AssetBundle build map file is an asset used to store assets to asset bundles relationship. "
            );
        }
        

        public GeneralSettingsTab() {
            Refresh();
        }

        public void Refresh()
        {
        }

//----------------------------------------------------------------------------------------------------------------------        
        public void Setup(VisualElement root) {
            root.Add(new Label("General Settings Content"));
            
        }
        

	}
}
