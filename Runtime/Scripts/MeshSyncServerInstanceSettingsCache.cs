using System;
using UnityEngine;

namespace Unity.MeshSync {
[Serializable]
public class MeshSyncServerInstanceSettingsCache {
    [SerializeField]
    public bool autostart;

    [SerializeField] public bool dirty;
}
}