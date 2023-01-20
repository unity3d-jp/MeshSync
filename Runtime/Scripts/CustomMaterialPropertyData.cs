using UnityEngine;

namespace Unity.MeshSync
{
    internal class CustomMaterialPropertyData : IMaterialPropertyData
    {
        public string name { get; set; }
        public int nameID { get; set; }
        public IMaterialPropertyData.Type type { get; set; }
        public int intValue { get; set; }
        public float floatValue { get; set; }
        public Vector4 vectorValue { get; set; }
        public Matrix4x4 matrixValue { get; set; }
        public IMaterialPropertyData.TextureRecord textureValue { get; set; }
        public int arrayLength { get; set; }
        public float[] floatArray { get; set; }
        public Vector4[] vectorArray { get; set; }
        public Matrix4x4[] matrixArray { get; set; }
    }
}
