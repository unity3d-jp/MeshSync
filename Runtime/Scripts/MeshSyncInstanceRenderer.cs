using System;
using System.Linq;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.SceneManagement;
using Object = UnityEngine.Object;
#if UNITY_EDITOR

#endif

namespace Unity.MeshSync{
    
    [ExecuteInEditMode]
    internal partial class MeshSyncInstanceRenderer : MonoBehaviour
    {
        internal BaseMeshSync m_server;
        internal InstanceRenderingInfo m_renderingInfo = new InstanceRenderingInfo();

        [SerializeField] private Matrix4x4[] m_transforms;

        private bool m_isUpdating = false;

        public void Init(BaseMeshSync ms)
        {
            m_server = ms;
            
            ms.onSceneUpdateEnd -= OnSceneUpdateEnd;
            ms.onSceneUpdateEnd += OnSceneUpdateEnd;
            
            ms.onSceneUpdateBegin -= OnSceneUpdateBegin;
            ms.onSceneUpdateBegin += OnSceneUpdateBegin;
        }

        void OnEnable()
        {
            if (RenderPipelineManager.currentPipeline == null)
            {
                Camera.onPreCull += OnCameraPreCull;
            }
            else
            {
                RenderPipelineManager.beginFrameRendering += OnBeginFrameRendering;
            }

            UpdateRenderingInfo(m_renderingInfo);
        }

        void OnDisable()
        {
            if (RenderPipelineManager.currentPipeline == null)
            {
                Camera.onPreCull -= OnCameraPreCull;
            }
            else
            {
                RenderPipelineManager.beginFrameRendering -= OnBeginFrameRendering;
            }
        }
        
        private void OnBeginFrameRendering(ScriptableRenderContext arg1, Camera[] cameras)
        {
            Draw(cameras);
        }

        private void OnCameraPreCull(Camera cam)
        {
            if (cam.name == "Preview Scene Camera")
                return;
            
            Camera[] cameras = {cam};
            Draw(cameras);
        }

        private void OnSceneUpdateBegin()
        {
            m_isUpdating = true;
        }

        private void OnSceneUpdateEnd()
        {
            m_isUpdating = false;
        }

        #region Events

        public void UpdateTransforms(Matrix4x4[] transforms)
        {
            m_transforms = transforms;
            m_renderingInfo.Instances = m_transforms;
        }

        private void UpdateInfoFromRenderer(InstanceRenderingInfo info)
        {
            if (TryGetComponent(out SkinnedMeshRenderer skinnedMeshRenderer))
            {
                info.Mesh = skinnedMeshRenderer.sharedMesh;
                info.Materials = skinnedMeshRenderer.sharedMaterials;
                info.GameObject = gameObject;
                info.Renderer = skinnedMeshRenderer;
                return;
            }
            
            if (!TryGetComponent(out MeshFilter filter))
            {
                Debug.LogWarningFormat("[MeshSync] No Mesh Filter for {0}", name);
                return;
            }

            if (!TryGetComponent(out MeshRenderer renderer))
            {
                Debug.LogWarningFormat("[MeshSync] No renderer for {0}", name);
                return;
            }
            
            info.Mesh = filter.sharedMesh;
            info.Materials = renderer.sharedMaterials;
            info.GameObject = gameObject;
            info.Renderer = renderer;
        }

       
        private void UpdateRenderingInfo(InstanceRenderingInfo info)
        {
            // Transforms
            info.Instances = m_transforms;

            // Renderer related info
            UpdateInfoFromRenderer(info);
            
            // Enable instancing in materials
            for (var i = 0; i < info.Materials.Length; i++)
            {
                info.Materials[i].enableInstancing = true;
            }
        }
        #endregion
        
        #region Rendering
        
        
        public void Draw(Camera[] cameras)
        {
            if (m_isUpdating)
                return;
            
            DoDraw(m_renderingInfo, cameras);
        }

        private void DoDraw(InstanceRenderingInfo entry, Camera[] cameras)
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
            m_renderingInfo = null;
        }
        
        #endregion
    }

}

