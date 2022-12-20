using System;
using UnityEngine;

namespace Unity.MeshSync {
[Serializable]
internal class PrefabHolder {
    public string     name;
    public GameObject prefab;

    public override string ToString() {
        return $"PrefabHolder: {name} {prefab}";
    }
}
} //end namespace