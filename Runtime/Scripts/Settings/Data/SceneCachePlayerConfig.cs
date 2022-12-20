﻿using System;
using UnityEngine;

namespace Unity.MeshSync {
[Serializable]
internal class SceneCachePlayerConfig : MeshSyncPlayerConfig {
    internal SceneCachePlayerConfig() {
    }

    internal SceneCachePlayerConfig(MeshSyncPlayerConfig meshSyncPlayerConfig) : base(meshSyncPlayerConfig) {
    }

//----------------------------------------------------------------------------------------------------------------------          

//----------------------------------------------------------------------------------------------------------------------    

#pragma warning disable 414
    [SerializeField] private int m_sceneCachePlayerConfigVersion = 1;
#pragma warning restore 414
}
} //end namespace