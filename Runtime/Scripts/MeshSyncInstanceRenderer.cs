using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

#if UNITY_EDITOR
using UnityEditor;
#endif

using UnityEngine;
using UnityEngine.Analytics;

namespace Unity.MeshSync{
    
    [ExecuteInEditMode]
    public class MeshSyncInstanceRenderer : MonoBehaviour
    {

        private BaseMeshSync ms;
        
        public void Init(BaseMeshSync ms)
        {
            this.ms = ms;
            ms.onUpdateEntity -= OnUpdateEntity;
            ms.onUpdateEntity += OnUpdateEntity;
        }

        private void OnDestroy()
        {
            if (ms == null)
                return;

            ms.onUpdateEntity -= OnUpdateEntity;
        }

        private List<MeshInstanceInfo> meshInstances = new List<MeshInstanceInfo>();

        private class MeshInstanceInfo
        {
            public Mesh Mesh;
            public List<Matrix4x4[]> Instances;
            public Material[] Materials;
        }

        //internal abstract class ModifierInfo
        //{
        //    public string Name { get; private set; }

        //    public virtual void Set(VariantData data)
        //    {
        //        Name = data.name;
        //    }

        //    public enum ModifierType
        //    {
        //        Float, 
        //        Int, 
        //        Vector
        //    }

        //    public ModifierType Type { get; protected set; }
        //}

        //internal class FloatModifierInfo : ModifierInfo
        //{
        //    public float Value;
        //    public float Min = Mathf.NegativeInfinity;
        //    public float Max = Mathf.Infinity;
        //    public override void Set(VariantData data)
        //    {
        //        base.Set(data);
        //        Value = data.floatValue;
        //        Type = ModifierType.Float;
        //    }
        //}

        //internal class IntModifierInfo : ModifierInfo
        //{
        //    public int Value;
        //    public int Min = int.MinValue;
        //    public int Max = int.MaxValue;
        //    public override void Set(VariantData data)
        //    {
        //        base.Set(data);
        //        Value = data.intValue;
        //        Type = ModifierType.Int;
        //    }
        //}

        //internal class VectorModifierInfo : ModifierInfo
        //{
        //    public Vector3 Value;
        //    public Vector3 Min = Vector3.negativeInfinity;
        //    public Vector3 Max = Vector3.positiveInfinity;
        //    public override void Set(VariantData data)
        //    {
        //        base.Set(data);
        //        Value = data.vector3Value;
        //        Type = ModifierType.Vector;
        //    }
        //}

        //internal class ModifierInfoFactory
        //{
        //    public static ModifierInfo Create(VariantData data)
        //    {
        //        switch (data.type)
        //        {
        //            case VariantData.Type.Float:
        //                return new FloatModifierInfo();
        //            case VariantData.Type.Float3:
        //                return new VectorModifierInfo();
        //            case VariantData.Type.Int:
        //                return new IntModifierInfo();
        //            default:
        //                throw new ArgumentOutOfRangeException();
        //        }
        //    }
        //}

        //internal Dictionary<GameObject, HashSet<ModifierInfo>> modifiersInfo = new Dictionary<GameObject, HashSet<ModifierInfo>>();

        //private void HandleModifiers(TransformData data, GameObject go)
        //{
        //    var modifiersProperty = data.FindUserProperty("modifiers");
        //    if (modifiersProperty.self == IntPtr.Zero)
        //    {
        //        return;
        //    }
            
        //    var modifierNames = modifiersProperty.stringValue.Split('\n');

        //    var index = 0;
        //    foreach (var modifierName in modifierNames)
        //    {

        //        if (string.IsNullOrEmpty(modifierName))
        //            continue;
                
        //        Debug.LogFormat("[MeshSync] Modifier{0}: {1}", index++, modifierName);

        //        var modifier = data.FindUserProperty(modifierName);

        //        if (modifier.self == IntPtr.Zero)
        //        {
        //            Debug.LogWarningFormat("[MeshSync] Modifier {0} found in manifest but does not exist in properties", modifierName);
        //            continue;
        //        }

        //        var modifierInfo = ModifierInfoFactory.Create(modifier);
        //        modifierInfo.Set(modifier);
        //        if (!modifiersInfo.TryGetValue(go, out HashSet<ModifierInfo> modifiers))
        //        {
        //            modifiers = new HashSet<ModifierInfo>();
        //            modifiersInfo.Add(go, modifiers);
        //        }

        //        modifiers.Add(modifierInfo);
        //    }
        //}
        
        private void OnUpdateEntity(GameObject obj, TransformData data)
        {
            //HandleModifiers(data, obj);
            HandleInstances(obj, data);

            // If Running on Editor, the app might be out of focus, need to force a draw as update will not be invoked
#if UNITY_EDITOR
            Draw();
            SceneView.RepaintAll();
#endif
        }

        private void HandleInstances(GameObject obj, TransformData data)
        {
            var instances = data.FindUserProperty("instances");

            if (instances.self == IntPtr.Zero)
                return;

            if (!obj.TryGetComponent(out MeshFilter meshFilter))
            {
                Debug.LogWarningFormat("Object {0} has instances but no MeshFilter", obj.name);
                return;
            }

            if (!obj.TryGetComponent(out MeshRenderer renderer))
            {
                Debug.LogWarningFormat("Object {0} has instance but no MeshRenderer", obj.name);
                return;
            }

            var mesh = meshFilter.sharedMesh;

            var entry = meshInstances.Find(x => x.Mesh == mesh);

            if (entry == null)
            {
                entry = new MeshInstanceInfo
                {
                    Mesh = mesh
                };
                meshInstances.Add(entry);
            }

            entry.Instances = DivideArrays(instances.matrixArray);
            entry.Materials = renderer.sharedMaterials;
            foreach (var mat in entry.Materials)
            {
                mat.enableInstancing = true;
            }
        }

        private List<Matrix4x4[]> DivideArrays(Matrix4x4[] arrays)
        {
            var result = new List<Matrix4x4[]>();
            var iterations = arrays.Length / 1023;
            for (var i = 0; i < iterations; i++)
            {
                var array = new Matrix4x4[1023];
                
                Array.Copy(
                    arrays, 
                    i * 1023, 
                    array, 
                    0, 
                    1023);
                
                result.Add(array);
            }

            var remainder = arrays.Length % 1023;
            if (remainder > 0)
            {
                var array = new Matrix4x4[remainder];
                
                Array.Copy(
                    arrays, 
                    iterations*1023, 
                    array, 
                    0, 
                    remainder);
                
                result.Add(array);
            }

            return result;
        }

        private void Update()
        {
            Draw();
        }

        private void Draw()
        {
            foreach (var entry in meshInstances)
            {
                RenderInstances(entry);
            }
        }

        private void RenderInstances(MeshInstanceInfo entry)
        {
            var mesh = entry.Mesh;
            var matrixBatches = entry.Instances;

            if (entry.Materials.Length == 0)
                return;
            
            for (var i = 0; i < mesh.subMeshCount; i++)
            {
                // Try to get the material in the same index position as the mesh
                // or the last material.
                
                var materialIndex = Mathf.Max(Mathf.Min(entry.Materials.Length -1, i), 0);
                var material = entry.Materials[materialIndex];
                for (var j = 0; j < matrixBatches.Count; j++)
                {
                    var batch = matrixBatches[j];
                    Graphics.DrawMeshInstanced(mesh, i, material, batch);
                }
            }
        }
    }
}

