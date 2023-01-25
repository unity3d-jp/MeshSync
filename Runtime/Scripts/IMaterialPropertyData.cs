using UnityEngine;

namespace Unity.MeshSync
{
    internal interface IMaterialPropertyData
    {
        public enum Type {
            Unknown,
            Int,
            Float,
            Vector,
            Matrix,
            Texture,
            String
        }
        public struct TextureRecord {
            public int     id;
            public bool    hasScaleOffset;
            public Vector2 scale, offset;
        }
        
        string name { get; }
        int nameID { get; }
        Type type { get; }
        int intValue { get; }
        float floatValue { get; }
        Vector4 vectorValue { get; }
        Matrix4x4 matrixValue { get; }
        TextureRecord textureValue { get; }
        int arrayLength { get; }
        float[] floatArray { get; }
        Vector4[] vectorArray { get; }
        Matrix4x4[] matrixArray { get; }
        string ToString();
    }
}
