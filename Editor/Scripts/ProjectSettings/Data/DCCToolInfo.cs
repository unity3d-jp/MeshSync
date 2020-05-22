﻿using System;
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
    /// The path to the DCC Tool application file
    /// </summary>
    public string         AppPath;

    /// <summary>
    /// The path to the icon of the DCC Tool
    /// </summary>
    public string         IconPath;
    
#pragma warning disable 414
    [SerializeField] private readonly int m_classVersion = 1;
#pragma warning restore 414
    
    
}

}


