using System;
using UnityEngine;

namespace Unity.MeshSync {
[Serializable]
internal class MeshSyncServerConfig : MeshSyncPlayerConfig {
    internal MeshSyncServerConfig() { }

    internal MeshSyncServerConfig(MeshSyncServerConfig other) : base(other)
    {
        SyncInstances        = other.SyncInstances;
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
#pragma warning disable 414
    [SerializeField] private int m_meshSyncServerConfigVersion = (int)MeshSyncServerConfigVersion.INITIAL;
#pragma warning restore 414

    private const int CUR_MESHSYNC_SERVER_CONFIG_VERSION = (int)MeshSyncServerConfigVersion.INITIAL; 

    enum MeshSyncServerConfigVersion {
        INITIAL = 1,
    }
    
    public bool SyncInstances = false; 
   

}
} //end namespace