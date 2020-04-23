using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;
using UnityEngine.UIElements;

namespace UnityEditor.MeshSync {
	
// Create a new type of Settings Asset.
class MyCustomSettings : ScriptableObject
{
    public const string k_MyCustomSettingsPath = "Assets/Editor/MyCustomSettings.asset";

    [SerializeField]
    private int m_Number;

    [SerializeField]
    private string m_SomeString;

    internal static MyCustomSettings GetOrCreateSettings()
    {
        var settings = AssetDatabase.LoadAssetAtPath<MyCustomSettings>(k_MyCustomSettingsPath);
        if (settings == null)
        {
            settings = ScriptableObject.CreateInstance<MyCustomSettings>();
            settings.m_Number = 42;
            settings.m_SomeString = "The answer to the universe";
            AssetDatabase.CreateAsset(settings, k_MyCustomSettingsPath);
            AssetDatabase.SaveAssets();
        }
        return settings;
    }

    internal static SerializedObject GetSerializedSettings()
    {
        return new SerializedObject(GetOrCreateSettings());
    }
}

// Register a SettingsProvider using IMGUI for the drawing framework:
static class MyCustomSettingsIMGUIRegister
{
    [SettingsProvider]
    public static SettingsProvider CreateMyCustomSettingsProvider()
    {
        // First parameter is the path in the Settings window.
        // Second parameter is the scope of this setting: it only appears in the Project Settings window.
        var provider = new SettingsProvider("Project/MyCustomIMGUISettings", SettingsScope.Project)
        {
            // By default the last token of the path is used as display name if no label is provided.
            label = "Custom IMGUI",
            // Create the SettingsProvider and initialize its drawing (IMGUI) function in place:
            guiHandler = (searchContext) =>
            {
                var settings = MyCustomSettings.GetSerializedSettings();
                EditorGUILayout.PropertyField(settings.FindProperty("m_Number"), new GUIContent("My Number"));
                EditorGUILayout.PropertyField(settings.FindProperty("m_SomeString"), new GUIContent("My String"));
            },

            // Populate the search keywords to enable smart search filtering and label highlighting:
            keywords = new HashSet<string>(new[] { "Number", "Some String" })
        };

        return provider;
    }
}

// Register a SettingsProvider using UIElements for the drawing framework:
static class MyCustomSettingsUIElementsRegister
{
    [SettingsProvider]
    public static SettingsProvider CreateMyCustomSettingsProvider()
    {
        // First parameter is the path in the Settings window.
        // Second parameter is the scope of this setting: it only appears in the Settings window for the Project scope.
        var provider = new SettingsProvider("Project/MyCustomUIElementsSettings", SettingsScope.Project)
        {
            label = "Custom UI Elements",
            // activateHandler is called when the user clicks on the Settings item in the Settings window.
            activateHandler = (searchContext, rootElement) =>
            {
                var settings = MyCustomSettings.GetSerializedSettings();

                // rootElement is a VisualElement. If you add any children to it, the OnGUI function
                // isn't called because the SettingsProvider uses the UIElements drawing framework.
                var styleSheet = AssetDatabase.LoadAssetAtPath<StyleSheet>("Assets/Editor/settings_ui.uss");
                rootElement.styleSheets.Add(styleSheet);
                var title = new Label()
                {
                    text = "Custom UI Elements"
                };
                title.AddToClassList("title");
                rootElement.Add(title);

                var properties = new VisualElement()
                {
                    style =
                    {
                        flexDirection = FlexDirection.Column
                    }
                };
                properties.AddToClassList("property-list");
                rootElement.Add(properties);

                var tf = new TextField()
                {
                    value = settings.FindProperty("m_SomeString").stringValue
                };
                tf.AddToClassList("property-value");
                properties.Add(tf);
            },

            // Populate the search keywords to enable smart search filtering and label highlighting:
            keywords = new HashSet<string>(new[] { "Number", "Some String" })
        };

        return provider;
    }
}

// Create MyCustomSettingsProvider by deriving from SettingsProvider:
class MyCustomSettingsProvider : SettingsProvider
{
    private SerializedObject m_CustomSettings;

    class Styles
    {
        public static GUIContent number = new GUIContent("My Number");
        public static GUIContent someString = new GUIContent("Some string");
    }

    const string k_MyCustomSettingsPath = "Assets/Editor/MyCustomSettings.asset";
    public MyCustomSettingsProvider(string path, SettingsScope scope = SettingsScope.User)
        : base(path, scope) {}

    public static bool IsSettingsAvailable()
    {
        return File.Exists(k_MyCustomSettingsPath);
    }

    public override void OnActivate(string searchContext, VisualElement rootElement)
    {
        // This function is called when the user clicks on the MyCustom element in the Settings window.
        m_CustomSettings = MyCustomSettings.GetSerializedSettings();
    }

    public override void OnGUI(string searchContext)
    {
        // Use IMGUI to display UI:
        EditorGUILayout.PropertyField(m_CustomSettings.FindProperty("m_Number"), Styles.number);
        EditorGUILayout.PropertyField(m_CustomSettings.FindProperty("m_SomeString"), Styles.someString);
    }

    // Register the SettingsProvider
    [SettingsProvider]
    public static SettingsProvider CreateMyCustomSettingsProvider()
    {
        if (IsSettingsAvailable())
        {
            var provider = new MyCustomSettingsProvider("Project/MyCustomSettingsProvider", SettingsScope.Project);

            // Automatically extract all keywords from the Styles.
            provider.keywords = GetSearchKeywordsFromGUIContentProperties<Styles>();
            return provider;
        }

        // Settings Asset doesn't exist yet; no need to display anything in the Settings window.
        return null;
    }
}

	//
	// class AssetGraphSettingsProvider : SettingsProvider
	// {
	// 	private AssetBundlesSettingsTab     m_abTab;
	// 	private ExecutionOrderSettingsTab   m_execTab;
	// 	private Mode m_mode;
	//
	// 	enum Mode : int {
	// 		AssetBundleSettings,
	// 		ExecutionOrderSettings
	// 	}
	// 	
	// 	public AssetGraphSettingsProvider(string path, SettingsScope scope = SettingsScope.Project)
	// 		: base(path, scope)
	// 	{
	// 		m_abTab = new AssetBundlesSettingsTab ();
	// 		m_execTab = new ExecutionOrderSettingsTab ();
	// 		m_mode = Mode.AssetBundleSettings;			
	// 	}
	//
	// 	public override void OnGUI(string searchContext)
	// 	{
	// 		DrawToolBar ();
	//
	// 		switch (m_mode) {
	// 			case Mode.AssetBundleSettings:
	// 				m_abTab.OnGUI ();
	// 				break;
	// 			case Mode.ExecutionOrderSettings:
	// 				m_execTab.OnGUI ();
	// 				break;
	// 		}
	// 	}
	// 	
	// 	
	// 	
	// 	private void DrawToolBar() {
	// 		GUILayout.BeginHorizontal();
	// 		GUILayout.FlexibleSpace();
	// 		float toolbarWidth = 300f;
	// 		string[] labels = new string[] { "Asset Bundles", "Execution Order" };
	// 		m_mode = (Mode)GUILayout.Toolbar((int)m_mode, labels, "LargeButton", GUILayout.Width(toolbarWidth) );
	// 		GUILayout.FlexibleSpace();
	// 		GUILayout.EndHorizontal();
	// 		GUILayout.Space(8f);
	// 	}
	// 	
	// 	class Styles
	// 	{
	// 		public static GUIContent number = new GUIContent("My Number");
	// 		public static GUIContent someString = new GUIContent("Some string");
	// 	}
	// 	
	// 	// Register the SettingsProvider
	// 	[SettingsProvider]
	// 	public static SettingsProvider CreateAssetGraphSettingsProvider() {
	// 		var provider = new AssetGraphSettingsProvider("Project/MeshSync", SettingsScope.Project);
	// 		
	// 		// Automatically extract all keywords from the Styles.
	// 		provider.keywords = GetSearchKeywordsFromGUIContentProperties<Styles>();			
	//
	// 		return provider;
	// 	}
	// }
}
