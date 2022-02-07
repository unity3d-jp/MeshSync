using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEditor;
using UnityEngine;

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
        
        private void OnUpdateEntity(GameObject obj, TransformData data)
        {
            
            var instances = data.FindUserProperty("instances");

            if (instances.self == IntPtr.Zero)
                return;

            var meshFilter = obj.GetComponent<MeshFilter>();

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

            var renderer = obj.GetComponent<MeshRenderer>();

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
            foreach (var entry in meshInstances)
            {
                RenderInstances(entry);
            }
        }

        private void RenderInstances(MeshInstanceInfo entry)
        {
            var mesh = entry.Mesh;
            var matrixBatches = entry.Instances;

            for (var i = 0; i < mesh.subMeshCount; i++)
            {
                for (var j = 0; j < matrixBatches.Count; j++)
                {
                    var batch = matrixBatches[j];
                    var material = entry.Materials.Last();
                    Graphics.DrawMeshInstanced(mesh, i, material, batch);
                    /*for (var k = 0; k < entry.Materials.Length; k++)
                    {
                        
                        var material = entry.Materials[k];
                        Graphics.DrawMeshInstanced(mesh, i, material, batch);
                    }*/
                }
            }
        }
    }
}

