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

    [SerializeField] internal int ClassVersion = 1;

}

} //end namespace