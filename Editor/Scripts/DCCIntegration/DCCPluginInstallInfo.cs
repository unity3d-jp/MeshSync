using System;
using UnityEngine;

namespace UnityEditor.MeshSync {

[Serializable]
internal class DCCPluginInstallInfo {

    internal DCCPluginInstallInfo(string pluginVersion) {
        PluginVersion = pluginVersion;
    }

//----------------------------------------------------------------------------------------------------------------------

    /// <summary>
    /// MeshSync plugin version installed for this DCC tool
    /// </summary>
    public string         PluginVersion;

    // ReSharper disable once NotAccessedField.Local
    [SerializeField] private int m_classVersion = 1;

}

} //end namespace