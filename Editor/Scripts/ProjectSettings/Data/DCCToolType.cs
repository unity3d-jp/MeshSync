﻿using System;

namespace Unity.MeshSync.Editor {

/// <summary>
/// DCC Tools which MeshSync provides integration support
/// </summary>
[Serializable]
public enum DCCToolType {
    AUTODESK_MAYA = 0,
    AUTODESK_3DSMAX,
    BLENDER,
    NUM_DCC_TOOL_TYPES,
//    AUTODESK_MOTION_BUILDER,
    
}

}


