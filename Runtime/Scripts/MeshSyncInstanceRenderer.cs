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
        
        private Dictionary<string, MeshInstanceInfo> m_instanceInfo = new Dictionary<string, MeshInstanceInfo>();


        public enum CameraMode
        {
            None = 0,
            SceneCameras = 1,
            GameCameras = 2,
            AllCameras = 3
        }

        private CameraMode m_cameraMode = CameraMode.GameCameras;

        public void Init(BaseMeshSync ms, CameraMode cameraMode = CameraMode.GameCameras)
        {
            m_server = ms;
            m_cameraMode = cameraMode;

#if UNITY_EDITOR
            EditorApplication.update -= Draw;
            EditorApplication.update += Draw;
            
            //To cover cases where the user adds cameras to the scene manually
            EditorApplication.hierarchyChanged -= OnHierarchyChanged;
            EditorApplication.hierarchyChanged += OnHierarchyChanged;
#endif
            
            RefreshCameraList();
            
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
        private void RefreshCameraListWithGameAndSceneCameras()
        {
            var gameViewCameras = Object.FindObjectsOfType<Camera>();
            var sceneViewCameras = SceneView.GetAllSceneCameras();
            m_cameras = new Camera[gameViewCameras.Length + sceneViewCameras.Length];

            for (var i = 0; i < gameViewCameras.Length; i++)
            {
                m_cameras[i] = gameViewCameras[i];
            }

            for (var i = 0; i < sceneViewCameras.Length; i++)
            {
                m_cameras[i + gameViewCameras.Length] = sceneViewCameras[i];
            }
        }
        
        private void OnHierarchyChanged()
        {
            RefreshCameraList();
        }
#endif

        private void RefreshCameraList()
        {
            RenderAllCameras();
            m_cameras = null;
#if UNITY_EDITOR
            if (m_cameraMode == CameraMode.AllCameras)
            {
                RefreshCameraListWithGameAndSceneCameras();
            }
            else if (m_cameraMode == CameraMode.GameCameras)
            {
                m_cameras = Object.FindObjectsOfType<Camera>();
            }
            else if (m_cameraMode == CameraMode.SceneCameras)
            {
                m_cameras = SceneView.GetAllSceneCameras();
            }
#else
            m_cameras = GameObject.FindObjectsOfType<Camera>();
#endif
        }

        #region Events

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

            m_instanceInfo.Remove(path);
            
            RepaintAfterChanges();
        }

        private void OnUpdateInstanceMesh(string path, GameObject go)
        {
            if (!this.m_instanceInfo.TryGetValue(path, out MeshInstanceInfo entry))
            {
                entry = new MeshInstanceInfo();
                this.m_instanceInfo.Add(path, entry);
            }
            
            UpdateEntryMeshMaterials(path, go, entry);
        }

        private void OnDeleteInstanceInfo(string path)
        {

            if (path == null)
                return;
            
            m_instanceInfo.Remove(path);
            
            RepaintAfterChanges();
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
            

            if (!m_instanceInfo.TryGetValue(path, out MeshInstanceInfo entry))
            {
                entry = new MeshInstanceInfo();
                
                m_instanceInfo.Add(path, entry);
            }

            UpdateEntryMeshMaterials(path, go, entry);

            entry.Instances = transforms;
            
            foreach (var mat in entry.Materials)
            {
                mat.enableInstancing = true;
            } 
            
            RepaintAfterChanges();
        }

        private void UpdateEntryMeshMaterials(string path, GameObject go, MeshInstanceInfo entry)
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
            entry.GameObject = go;
            entry.Renderer = renderer;
            entry.Root = m_server.transform;
        }
        #endregion
        
        #region Rendering
        public void Draw()
        {
            foreach (var entry in m_instanceInfo)
            {
                DrawInstances(entry.Value);
            }
        }

        private void DrawInstances(MeshInstanceInfo entry)
        {
            if (entry.Mesh == null || entry.DividedInstances == null || entry.Materials == null)
            {
                return;
            }
            
            var mesh = entry.Mesh;
            
            entry.UpdateDividedInstances();
            var matrixBatches = entry.DividedInstances;

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
                    
                    
                    DrawOnCameras(
                        mesh, 
                        i,
                        material,
                        batch, 
                        entry.Layer, 
                        entry.ReceiveShadows, 
                        entry.ShadowCastingMode);
                }
            }
        }
        
        private void DrawOnCameras(
            Mesh mesh,
            int submeshIndex,
            Material material,
            Matrix4x4[] matrices,
            int layer,
            bool receiveShadows,
            ShadowCastingMode shadowCastingMode)
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
                    castShadows:shadowCastingMode, 
                    receiveShadows:receiveShadows,
                    layer:layer, 
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
        #endregion
    }


}

