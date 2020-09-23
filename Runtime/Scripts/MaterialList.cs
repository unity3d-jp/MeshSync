using System;
using System.Collections.Generic;
using UnityEngine;

namespace Unity.MeshSync
{
    internal class MaterialList : ScriptableObject
    {
        [Serializable]
        public class Node
        {
            public string path;
            public Material[] materials;
        }

        [SerializeField] public List<MaterialHolder> materials = new List<MaterialHolder>();
        [SerializeField] public List<Node> nodes = new List<Node>();
    }
}
