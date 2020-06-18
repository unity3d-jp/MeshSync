using System;
using UnityEngine;

namespace Unity.MeshSync.Editor {

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
        AppPath = other.AppPath;
    }

//----------------------------------------------------------------------------------------------------------------------    

    internal string GetDescription() {
        string desc = null;
        switch (Type) {
            case DCCToolType.AUTODESK_MAYA: {
                desc = "Maya ";
                break;
            }
            case DCCToolType.AUTODESK_3DSMAX: {
                desc = "3DS Max ";
                break;
            }
            case DCCToolType.BLENDER: {
                desc = "Blender ";
                break;
            }
            default: {
                throw new NotImplementedException();
            }
        }

        return desc + DCCToolVersion;
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
    /// The path to the DCC Tool application file
    /// </summary>
    public string         AppPath;

    /// <summary>
    /// The path to the icon of the DCC Tool
    /// </summary>
    public string         IconPath;
    
    [SerializeField] internal readonly int ClassVersion = 1;
    
    
}

}


