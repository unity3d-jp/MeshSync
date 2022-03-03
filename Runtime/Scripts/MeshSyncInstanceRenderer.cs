using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

#if UNITY_EDITOR
using UnityEditor;
using UnityEditor.Rendering;

#endif

namespace Unity.MeshSync{
    
    public class MeshSyncInstanceRenderer
    {
        private BaseMeshSync m_server;

        private Camera m_camera;

        public void Init(BaseMeshSync ms)
        {
            this.m_server = ms;

            if (Application.isEditor)
            {
                var go = GameObject.Find("SceneCamera");
                if (go != null)
                {
                    m_camera = go.GetComponent<Camera>();
                }
            }

            EditorApplication.update -= Draw;
            EditorApplication.update += Draw;
            
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
            
            RepaintAfterChanges();
        }

        private void OnUpdateInstanceMesh(string path, GameObject go)
        {
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
                    
                    Graphics.DrawMeshInstanced(
                        mesh:mesh,
                        submeshIndex:i, 
                        material:material, 
                        matrices:batch, 
                        count:batch.Length, 
                        properties:null, 
                        castShadows:ShadowCastingMode.On, 
                        receiveShadows:true,
                        layer:0, 
                        camera:m_camera);
                }
            }
        }
        
        private void RepaintAfterChanges()
        {
#if UNITY_EDITOR
            m_camera.Render();
            Draw();
#endif
        }
    }
}

