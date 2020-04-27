using System;

namespace UnityEditor.MeshSync {
    
[Serializable]
internal class DCCToolInfo {

    internal DCCToolInfo(DCCToolType type, string version) {
        Type = type;
        Version = version;
    }
    
//----------------------------------------------------------------------------------------------------------------------    
    public DCCToolType    Type;    //DCC Tool Type
    public string         Version; //DCC Tool Version
    public string         PluginVersion;
}

}


