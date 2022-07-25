
using System.Runtime.CompilerServices;
using UnityEditor;
using UnityEngine;
[FilePath("Packages/com.unity.meshsync/AutoServerCreationSettings.txt", FilePathAttribute.Location.ProjectFolder)]
internal class AutoServerCreationSettings : ScriptableSingleton<AutoServerCreationSettings> {
    
    [SerializeField]
    private bool m_hasPromptedUser = false;

    public bool HasPromptedUser {
        get { return m_hasPromptedUser; }
        set {
            m_hasPromptedUser = value;
            Save(true);
        }
    }
}
