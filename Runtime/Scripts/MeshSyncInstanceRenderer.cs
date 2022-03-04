using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.MeshSync{
    
    public class MeshSyncInstanceRenderer
    {
        private BaseMeshSync m_server;

        private Camera[] m_cameras;

        private enum CameraMode
        {
            None = 0,
            SceneCameras = 1,
            GameCameras = 2,
        }

        private CameraMode m_cameraMode = CameraMode.GameCameras;

        public void Init(BaseMeshSync ms)
        {
            this.m_server = ms;

#if UNITY_EDITOR
            EditorApplication.update -= Draw;
            EditorApplication.update += Draw;
            
            //To cover cases where the user adds cameras to the scene manually
            EditorApplication.hierarchyChanged -= OnHierarchyChanged;
            EditorApplication.hierarchyChanged += OnHierarchyChanged;

            if (m_cameraMode == CameraMode.GameCameras)
            {
                m_cameras = GameObject.FindObjectsOfType<Camera>();
            }
            else if (m_cameraMode == CameraMode.SceneCameras)
            {
                m_cameras = SceneView.GetAllSceneCameras();
            }
            
            if (m_cameras != null)
            {
                Debug.LogFormat("[MeshSync] Found {0} cameras", m_cameras.Length);
            }
#else
                m_cameras = GameObject.FindObjectsOfType<Camera>();
#endif
            
            ms.onUpdateInstanceInfo -= OnUpdateInstanceInfo;
            ms.onUpdateInstanceInfo += OnUpdateInstanceInfo;
            ms.onDeleteInstanceInfo -= OnDeleteInstanceInfo;
            ms.onDeleteInstanceInfo += OnDeleteInstanceInfo;
            ms.onUpdateInstanceMesh -= OnUpdateInstanceMesh;
            ms.onUpdateInstanceMesh += OnUpdateInstanceMesh;
            ms.onDeleteInstanceMesh -= OnDeleteInstanceMesh;
            ms.onDeleteInstanceMesh += OnDeleteInstanceMesh;
            ms.onUpdateEntity -= OnUpdateEntity;
            ms.onUpdateEntity += OnUpdateEntity;
            ms.onDeleteEntity -= OnDeleteEntity;
        }



#if UNITY_EDITOR
        private void OnHierarchyChanged()
        {
            RefreshCameraList();
        }
#endif

        private void RefreshCameraList()
        {
            if (m_cameraMode == CameraMode.GameCameras)
            {
                RenderAllCameras();
                m_cameras = GameObject.FindObjectsOfType<Camera>();
            }
        }
        
        private void OnDeleteEntity(GameObject obj)
        {
            if (obj.TryGetComponent(out Camera _))
            {
                RefreshCameraList();
            }
        }

        private void OnUpdateEntity(GameObject obj, TransformData data)
        {
            if (data.entityType  == EntityType.Camera)
            {
                RefreshCameraList();
            }
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
                    
                    DrawAllCameras(mesh, i, material, batch);
                }
            }
        }

        private void DrawAllCameras(Mesh mesh,int submeshIndex, Material material, Matrix4x4[] matrices)
        {
            if (m_cameras == null)
                return;
            
            for (var i = 0; i < m_cameras.Length; i++)
            {
                var camera = m_cameras[i];
                if (camera == null)
                    continue;
                
                Graphics.DrawMeshInstanced(
                    mesh:mesh,
                    submeshIndex:submeshIndex, 
                    material:material, 
                    matrices:matrices, 
                    count:matrices.Length, 
                    properties:null, 
                    castShadows:ShadowCastingMode.On, 
                    receiveShadows:true,
                    layer:0, 
                    camera:camera);
            }
        }

        private void RenderAllCameras()
        {
            if (m_cameras == null)
                return;
            
            for (var i = 0; i < m_cameras.Length; i++)
            {
                var camera = m_cameras[i];
                if (camera == null)
                    continue;
                
                camera.Render();
            }
        }
        
        
        private void RepaintAfterChanges()
        {
            if (Application.isEditor && !Application.isPlaying)
            {
                RenderAllCameras();
            }

            Draw();
        }

        public void Clear()
        {
            RenderAllCameras();
            m_cameras = null;
            m_server = null;
            m_cameraMode = CameraMode.None;
        }
    }
}

