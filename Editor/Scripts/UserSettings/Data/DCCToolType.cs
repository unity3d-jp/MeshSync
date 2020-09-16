using System;

namespace Unity.MeshSync.Editor {

/// <summary>
/// DCC Tools for which MeshSync provides integration support
/// </summary>
[Serializable]
public enum DCCToolType {
    /// <summary>
    /// Maya from Autodesk  
    /// </summary>
    AUTODESK_MAYA = 0,

    /// <summary>
    /// 3dsMax from Autodesk  
    /// </summary>
    AUTODESK_3DSMAX,

    /// <summary>
    /// Blender  
    /// </summary>
    BLENDER,
    
    /// <summary>
    /// The number of supported DCC tools  
    /// </summary>
    NUM_DCC_TOOL_TYPES,
//    AUTODESK_MOTION_BUILDER,
    
}

}


