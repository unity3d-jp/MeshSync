using System;
using UnityEngine;

namespace Unity.MeshSync {
[Serializable]
internal class MeshSyncServerConfig : MeshSyncPlayerConfig {

    
#pragma warning disable 414
    [SerializeField] private int m_meshSyncServerConfigVersion = (int)MeshSyncPlayerConfigVersion.INITIAL;
#pragma warning restore 414

    private const int CUR_MESHSYNC_SERVER_CONFIG_VERSION = (int)MeshSyncPlayerConfigVersion.INITIAL; 

    enum MeshSyncPlayerConfigVersion {
        INITIAL = 1,
    }
   

}
} //end namespace