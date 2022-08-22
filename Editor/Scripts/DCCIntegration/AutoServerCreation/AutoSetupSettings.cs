using UnityEditor;
using UnityEngine;

[FilePath("Assets/MeshSyncAssets/AutoSetupSettings.txt", FilePathAttribute.Location.ProjectFolder)]
internal class AutoSetupSettings : ScriptableSingleton<AutoSetupSettings> {

    [SerializeField] private ushort m_port;

    internal ushort Port => m_port;

    internal void Save() {
        Save(true);
    }
}
