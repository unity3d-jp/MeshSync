using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class MaterialHolder {
    public int      id;
    public string   name;
    public int      index;
    public Color    color = Color.white;
    public Material material;
    
    //[TODO-sin: 2021-11-4] Remove in 2022.
    [Obsolete] public int  materialIID;
    
    public bool     ShouldApplyMaterialData = true;

}

} //end namespace
