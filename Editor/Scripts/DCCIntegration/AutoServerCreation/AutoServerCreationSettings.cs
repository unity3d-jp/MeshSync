
using UnityEditor;
using UnityEditorInternal;
using UnityEngine;
[FilePath("Assets/MeshSyncAssets/AutoServerCreationSettings.txt", FilePathAttribute.Location.ProjectFolder)]
internal class AutoServerCreationSettings : ScriptableSingleton<AutoServerCreationSettings> {
    
    /// <remarks>
    /// Set default to true so we don't show this to users
    /// when installing using the Unity package manager 
    /// </remarks>>
    [SerializeField]
    public bool m_hasPromptedUser = true;

    public bool HasPromptedUser {
        get { return m_hasPromptedUser; }
        set {
            m_hasPromptedUser = value;
            Save(true);
        }
    }
}
