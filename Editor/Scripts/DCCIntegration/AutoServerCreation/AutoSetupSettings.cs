using UnityEditor;
using UnityEngine;

[FilePath("Assets/MeshSyncAssets/AutoSetupSettings.yml", FilePathAttribute.Location.ProjectFolder)]
internal class AutoSetupSettings : ScriptableSingleton<AutoSetupSettings> {

    [SerializeField] private ushort m_port = 8081;

    internal ushort Port {
        get {return m_port; }
        set {
            
            if (m_port == value)
                return;
            
            m_port = value;
            Save(true);
        }
    }
}
