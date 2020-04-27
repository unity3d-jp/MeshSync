using System;

namespace UnityEditor.MeshSync {

/// <summary>
/// Basic information about DCCTool
/// </summary>
[Serializable]
public class DCCToolInfo {

    internal DCCToolInfo(DCCToolType type, string version) {
        Type = type;
        Version = version;
    }

    internal DCCToolInfo(DCCToolInfo other)  {
        Type = other.Type;
        Version = other.Version;
        PluginVersion = other.PluginVersion;
        AppPath = other.AppPath;
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    /// <summary>
    /// The type of DCC Tool
    /// </summary>
    public DCCToolType    Type;    //DCC Tool Type
    
    /// <summary>
    /// The version of DCC Tool
    /// </summary>
    public string         Version; //DCC Tool Version
    
    /// <summary>
    /// MeshSync plugin version installed for this DCC tool
    /// </summary>
    public string         PluginVersion;
    
    /// <summary>
    /// The path to the DCC Tool application file
    /// </summary>
    public string         AppPath;
}

}


