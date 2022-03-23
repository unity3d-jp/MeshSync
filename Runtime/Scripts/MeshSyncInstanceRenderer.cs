using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;
using Object = UnityEngine.Object;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.MeshSync{
    
    internal class MeshSyncInstanceRenderer
    {
        private BaseMeshSync m_server;
        
        private Dictionary<string, MeshInstanceInfo> m_instanceInfo = new Dictionary<string, MeshInstanceInfo>();

        private bool m_isUpdating = false;
        
        
        public void Init(
            BaseMeshSync ms, 
            Dictionary<string, InstanceInfoRecord> records = null)
        {
            m_server = ms;

            ms.onUpdateInstanceInfo -= OnUpdateInstanceInfo;
            ms.onUpdateInstanceInfo += OnUpdateInstanceInfo;
            ms.onDeleteInstanceInfo -= OnDeleteInstanceInfo;
            ms.onDeleteInstanceInfo += OnDeleteInstanceInfo;
            ms.onUpdateInstanceMesh -= OnUpdateInstanceMesh;
            ms.onUpdateInstanceMesh += OnUpdateInstanceMesh;
            ms.onDeleteInstanceMesh -= OnDeleteInstanceMesh;
            ms.onDeleteInstanceMesh += OnDeleteInstanceMesh;
            ms.onSceneUpdateEnd -= OnSceneUpdateEnd;
            ms.onSceneUpdateEnd += OnSceneUpdateEnd;
            ms.onSceneUpdateBegin -= OnSceneUpdateBegin;
            ms.onSceneUpdateBegin += OnSceneUpdateBegin;
            
            LoadData(records);
        }

        private void OnSceneUpdateBegin()
        {
            m_isUpdating = true;
        }

        private void LoadData(Dictionary<string, InstanceInfoRecord> records)
        {
            if (records == null)
                return;
            
            m_instanceInfo.Clear();
            
            foreach (var record in records)
            {
                var key = record.Key;
                var value = record.Value;
                
                OnUpdateInstanceInfo(key, value.go, value.transforms);
            }
        }

        private void OnSceneUpdateEnd()
        {
            m_isUpdating = false;
        }

        #region Events

        private void OnDeleteInstanceMesh(string path)
        {
            if (path == null)
                return;

            m_instanceInfo.Remove(path);
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

            if (!UpdateEntryMeshMaterials(path, go, entry))
            {
                return;
            }

            entry.Instances = transforms;
            
            foreach (var mat in entry.Materials)
            {
                mat.enableInstancing = true;
            }
        }

        private bool UpdateEntryMeshMaterials(string path, GameObject go, MeshInstanceInfo entry)
        {
            if (go.TryGetComponent(out SkinnedMeshRenderer skinnedMeshRenderer))
            {
                entry.Mesh = skinnedMeshRenderer.sharedMesh;
                entry.Materials = skinnedMeshRenderer.sharedMaterials;
                entry.GameObject = go;
                entry.Renderer = skinnedMeshRenderer;
                entry.Root = m_server.transform;

                return true;
            }
            
            if (!go.TryGetComponent(out MeshFilter filter))
            {
                Debug.LogWarningFormat("[MeshSync] No Mesh Filter for {0}", path);
                return false;
            }

            if (!go.TryGetComponent(out MeshRenderer renderer))
            {
                Debug.LogWarningFormat("[MeshSync] No renderer for {0}", path);
                return false;
            }

            entry.Mesh = filter.sharedMesh;
            entry.Materials = renderer.sharedMaterials;
            entry.GameObject = go;
            entry.Renderer = renderer;
            entry.Root = m_server.transform;
            
            return true;
        }
        #endregion
        
        #region Rendering
        
        
        public void Draw(Camera[] cameras)
        {
            if (m_isUpdating)
                return;
            
            foreach (var entry in m_instanceInfo)
            {
                DrawInstances(entry.Value, cameras);
            }
        }

        private void DrawInstances(MeshInstanceInfo entry, Camera[] cameras)
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
                        cameras,
                        mesh, 
                        i,
                        material,
                        batch, 
                        entry.Layer, 
                        entry.ReceiveShadows, 
                        entry.ShadowCastingMode, 
                        entry.LightProbeUsage, 
                        entry.LightProbeProxyVolume);
                }
            }
        }
        
        private void DrawOnCameras(
            Camera[] cameras,
            Mesh mesh,
            int submeshIndex,
            Material material,
            Matrix4x4[] matrices,
            int layer,
            bool receiveShadows,
            ShadowCastingMode shadowCastingMode,
            LightProbeUsage lightProbeUsage, 
            LightProbeProxyVolume lightProbeProxyVolume)
        {
            if (cameras == null)
                return;
            
            for (var i = 0; i < cameras.Length; i++)
            {
                var camera = cameras[i];
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
                    camera:camera,
                    lightProbeUsage:lightProbeUsage, 
                    lightProbeProxyVolume:lightProbeProxyVolume);
            }
        }
        
        
        public void Clear()
        {
            m_server = null;
            m_instanceInfo.Clear();
        }
        
        #endregion
    }


}

