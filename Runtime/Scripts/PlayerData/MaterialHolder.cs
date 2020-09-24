using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class MaterialHolder {
    public int      id;
    public string   name;
    public int      index;
    public string   shader;
    public Color    color = Color.white;
    public Material material;
    public int      materialIID;
}

} //end namespace
