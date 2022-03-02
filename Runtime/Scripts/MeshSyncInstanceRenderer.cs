using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using UnityEngine;
using UnityEngine.PlayerLoop;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.MeshSync{
    
    public class MeshSyncInstanceRenderer
    {
        private BaseMeshSync ms;
        
        public void Init(BaseMeshSync ms)
        {
            this.ms = ms;

            ms.onUpdateInstanceInfo -= OnUpdateInstanceInfo;
            ms.onUpdateInstanceInfo += OnUpdateInstanceInfo;
            ms.onDeleteInstanceInfo -= OnDeleteInstanceInfo;
            ms.onDeleteInstanceInfo += OnDeleteInstanceInfo;
            ms.onUpdateInstanceMesh -= OnUpdateInstanceMesh;
            ms.onUpdateInstanceMesh += OnUpdateInstanceMesh;
            ms.onDeleteInstanceMesh -= OnDeleteInstanceMesh;
            ms.onDeleteInstanceMesh += OnDeleteInstanceMesh;
        }

        private void OnDeleteInstanceMesh(string path)
        {
            if (path == null)
                return;

            meshInstances.Remove(path);
            
            Debug.LogFormat("MS: DELETE INST MESH {0}", path);
            RepaintAfterChanges();
        }

        private void OnUpdateInstanceMesh(string path, GameObject go)
        {
            Debug.LogFormat("MS: UPDATE INST MESH {0}", path);
            if (!this.meshInstances.TryGetValue(path, out MeshInstanceInfo entry))
            {
                entry = new MeshInstanceInfo();
                this.meshInstances.Add(path, entry);
            }
            
            UpdateEntryMeshMaterials(path, go, entry);
        }

        private void OnDeleteInstanceInfo(string path)
        {

            if (path == null)
                return;
            
            meshInstances.Remove(path);
            Debug.LogFormat("MS: DELETE INST INFO {0}", path);
            
            
            RepaintAfterChanges();
        }

        private Dictionary<string, MeshInstanceInfo> meshInstances = new Dictionary<string, MeshInstanceInfo>();

        private class MeshInstanceInfo
        {
            public Mesh Mesh;
            public List<Matrix4x4[]> Instances;
            public Material[] Materials;
        }
        
        private void OnUpdateInstanceInfo(
            string path,
            GameObject go,
            Matrix4x4[] transforms)
        {
            Debug.LogFormat("MS: UPDATE INST INFO {0}", path);
            
            if (go == null)
            {
                Debug.LogWarningFormat("[MeshSync] No Gameobject found: {0}", path);
                return;
            }
            

            if (!meshInstances.TryGetValue(path, out MeshInstanceInfo entry))
            {
                entry = new MeshInstanceInfo();
                
                meshInstances.Add(path, entry);
            }

            UpdateEntryMeshMaterials(path, go, entry);

            entry.Instances = DivideArrays(transforms);
            
            foreach (var mat in entry.Materials)
            {
                mat.enableInstancing = true;
            } 
            
            RepaintAfterChanges();
        }

        private static void UpdateEntryMeshMaterials(string path, GameObject go, MeshInstanceInfo entry)
        {
            if (!go.TryGetComponent(out MeshFilter filter))
            {
                Debug.LogWarningFormat("[MeshSync] No Mesh Filter for {0}", path);
            }

            if (!go.TryGetComponent(out MeshRenderer renderer))
            {
                Debug.LogWarningFormat("[MeshSync] No renderer for {0}", path);
            }

            entry.Mesh = filter.sharedMesh;

            entry.Materials = renderer.sharedMaterials;
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
        
        public void Draw()
        {
            foreach (var entry in meshInstances)
            {
                RenderInstances(entry.Value);
            }
        }

        private void RenderInstances(MeshInstanceInfo entry)
        {
            if (entry.Mesh == null || entry.Instances == null || entry.Materials == null)
            {
                return;
            }
            
            var mesh = entry.Mesh;
            var matrixBatches = entry.Instances;

            if (entry.Materials.Length == 0)
                return;
            
            for (var i = 0; i < mesh.subMeshCount; i++)
            {
                // Try to get the material in the same index position as the mesh
                // or the last material.
                var materialIndex = Mathf.Clamp(i, 0, entry.Materials.Length -1);
                
                var material = entry.Materials[materialIndex];
                for (var j = 0; j < matrixBatches.Count; j++)
                {
                    var batch = matrixBatches[j];
                    Graphics.DrawMeshInstanced(mesh, i, material, batch);
                }
            }
        }

        private void RepaintAfterChanges()
        {
#if UNITY_EDITOR
            SceneView.RepaintAll();
            Draw();
            SceneView.RepaintAll();
#endif     
        }
    }
}

