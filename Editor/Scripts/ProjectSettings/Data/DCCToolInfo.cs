using System;

namespace UnityEditor.MeshSync {
    
[Serializable]
internal class DCCToolInfo {

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
    public DCCToolType    Type;    //DCC Tool Type
    public string         Version; //DCC Tool Version
    public string         PluginVersion;
    public string         AppPath;
}

}


