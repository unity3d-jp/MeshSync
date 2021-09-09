using System;
using UnityEngine;

namespace Unity.MeshSync {
[Serializable]
internal class SceneCachePlayerConfig : MeshSyncPlayerConfig{

    internal SceneCachePlayerConfig() { }
    internal SceneCachePlayerConfig(MeshSyncPlayerConfig meshSyncPlayerConfig) : base(meshSyncPlayerConfig) { }
    
//----------------------------------------------------------------------------------------------------------------------       
    
    //Timeline
    internal int TimelineSnapToFrame = (int) SnapToFrame.NONE; 
    
//----------------------------------------------------------------------------------------------------------------------    
    
#pragma warning disable 414       
    [SerializeField] private readonly int m_sceneCachePlayerConfigVersion = 1;
#pragma warning restore 414    
}
} //end namespace