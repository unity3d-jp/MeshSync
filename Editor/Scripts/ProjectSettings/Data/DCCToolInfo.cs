using System;
using UnityEngine;

namespace UnityEditor.MeshSync {

/// <summary>
/// Basic information about DCCTool
/// </summary>
[Serializable]
public class DCCToolInfo {

    internal DCCToolInfo(DCCToolType type, string dccToolVersion) {
        Type = type;
        DCCToolVersion = dccToolVersion;
    }

    internal DCCToolInfo(DCCToolInfo other)  {
        Type = other.Type;
        DCCToolVersion = other.DCCToolVersion;
        PluginVersion = other.PluginVersion;
        AppPath = other.AppPath;
    }

//----------------------------------------------------------------------------------------------------------------------    

    internal string GetDescription() {
        string desc = null;
        switch (Type) {
            case DCCToolType.AUTODESK_MAYA: {
                desc = "Maya " + DCCToolVersion;
                break;
            }
            case DCCToolType.AUTODESK_3DSMAX: {
                desc = "3DS Max " + DCCToolVersion;
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
    public string         DCCToolVersion; //DCC Tool Version
    
    /// <summary>
    /// MeshSync plugin version installed for this DCC tool
    /// </summary>
    public string         PluginVersion;
    
    /// <summary>
    /// The path to the DCC Tool application file
    /// </summary>
    public string         AppPath;

    [SerializeField] internal int ClassVersion = 1;
    
    
}

}


