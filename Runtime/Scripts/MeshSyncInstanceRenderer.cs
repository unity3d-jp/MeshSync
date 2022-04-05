﻿#if UNITY_EDITOR
using UnityEditor.Experimental.SceneManagement;
#endif

using UnityEngine;
using UnityEngine.Rendering;

namespace Unity.MeshSync{
    
    [ExecuteInEditMode]
    internal class MeshSyncInstanceRenderer : MonoBehaviour
    {
        private InstanceRenderingInfo m_renderingInfo = new InstanceRenderingInfo();

        [HideInInspector]
        [SerializeField] private Matrix4x4[] m_transforms;
        [SerializeField] internal GameObject m_reference; 
        
        void OnEnable()
        {
        
            if (GraphicsSettings.currentRenderPipeline == null)
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
            if (GraphicsSettings.currentRenderPipeline == null)
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
            if (!IsInPrefabStage())
                return;
            
            Draw(cameras);
        }

        private void OnCameraPreCull(Camera cam)
        {
            if (cam.cameraType == CameraType.Preview)
                return;
            
            if (!IsInPrefabStage())
                return;
            
            Camera[] cameras = {cam};
            Draw(cameras);
        }
        
        
        private bool IsInPrefabStage()
        {
#if UNITY_EDITOR
            var stage = PrefabStageUtility.GetCurrentPrefabStage();
            return stage == null || stage.IsPartOfPrefabContents(gameObject);
#else
            return true;
#endif
        }

        #region Events

        public void UpdateAll(Matrix4x4[] transforms, GameObject go)
        {
            m_transforms = transforms;
            m_reference = go;
            
            UpdateRenderingInfo(m_renderingInfo);
        }

        public void UpdateReference(GameObject go)
        {
            m_reference = go;
            UpdateRenderingInfo(m_renderingInfo);
        }

       
        private void UpdateRenderingInfo(InstanceRenderingInfo info)
        {
            // Transforms
            info.Instances = m_transforms;
            
            // Reference
            info.GameObject = m_reference;
        }
        #endregion
        
        #region Rendering
        
        
        public void Draw(Camera[] cameras)
        {
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
        
        #endregion
    }

}

