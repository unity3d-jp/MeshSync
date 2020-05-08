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

    internal string GetDescription() {
        string desc = null;
        switch (Type) {
            case DCCToolType.AUTODESK_MAYA: {
                desc = "Maya " + Version;
                break;
            }
            case DCCToolType.AUTODESK_3DSMAX: {
                desc = "3DS Max " + Version;
                break;
            }
            default: {
                desc = "";
                break;
            }
        }

        return desc;
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


