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
            Directory.CreateDirectory(k_MyCustomSettingsPath);
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


// Register a SettingsProvider using UIElements for the drawing framework:
class MeshSyncSettingsProvider : SettingsProvider {
	private static VisualElement m_rootElement;

	private static int temp = 0;

	
	public MeshSyncSettingsProvider(string path, SettingsScope scope = SettingsScope.Project) : base(path,scope) {
		// activateHandler is called when the user clicks on the Settings item in the Settings window.
		activateHandler = (searchContext, rootElement) =>
		{
			m_rootElement = rootElement;
			var settings = MyCustomSettings.GetSerializedSettings();

			// rootElement is a VisualElement. If you add any children to it, the OnGUI function
			// isn't called because the SettingsProvider uses the UIElements drawing framework.
			//var styleSheet = AssetDatabase.LoadAssetAtPath<StyleSheet>("Assets/Editor/settings_ui.uss");
			//rootElement.styleSheets.Add(styleSheet);
			var title = new Label() {
				text = "Custom UI Elements"
			};
			title.AddToClassList("title");
			title.RegisterCallback<MouseDownEvent>(OnMouseDownEvent);

			rootElement.Add(title);

			var properties = new VisualElement() {
				style = {
					flexDirection = FlexDirection.Column
				}
			};
			properties.AddToClassList("property-list");
			rootElement.Add(properties);

			var tf = new TextField() {
				value = settings.FindProperty("m_SomeString").stringValue
			};
			tf.AddToClassList("property-value");
			properties.Add(tf);
		};

		// Populate the search keywords to enable smart search filtering and label highlighting:
		keywords = new HashSet<string>(new[] {"Number", "Some String"});
		
		Debug.Log("In constructor. Hello");

	}

	
	static void OnMouseDownEvent(MouseEventBase<MouseDownEvent> evt)
	{
		Debug.Log("Mouse Down " + evt + " in " + evt.propagationPhase + " for target " + evt.target);
		Debug.Log("RootElement: " + m_rootElement);
		
		
		var title = new Label()
		{
			text = "Custom UI Elements"
		};
		title.AddToClassList("title");
		m_rootElement.Add(title);
		++temp;

		if (temp >= 5) {
			m_rootElement.Clear();
		}

	}	
	
	
//----------------------------------------------------------------------------------------------------------------------

    [SettingsProvider]
    public static SettingsProvider CreateMeshSyncSettingsProvider() {
	    MeshSyncSettingsProvider provider = new MeshSyncSettingsProvider("Project/MeshSync", SettingsScope.Project);
	    return provider;
    }
}

//----------------------------------------------------------------------------------------------------------------------

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
