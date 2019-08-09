using System.Collections.Generic;
using UnityEngine;

namespace UTJ.MeshSync
{
    public class MaterialList : ScriptableObject
    {

        [SerializeField] public List<MeshSyncPlayer.MaterialHolder> materials;
    }
}
