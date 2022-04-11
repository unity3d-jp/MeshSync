#if UNITY_EDITOR
using UnityEditor.Experimental.SceneManagement;
#endif
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

namespace Unity.MeshSync{
    
    [ExecuteInEditMode]
    internal class MeshSyncInstanceRenderer : MonoBehaviour
    {
        private readonly InstanceRenderingInfo m_renderingInfo = new InstanceRenderingInfo();

        [HideInInspector]
        [SerializeField] private Matrix4x4[] transforms;
        [SerializeField] private GameObject reference; 
        
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
            
            Draw(m_renderingInfo, cameras);
        }

        private void OnCameraPreCull(Camera targetCamera)
        {
            if (targetCamera.cameraType == CameraType.Preview)
                return;
            
            if (!IsInPrefabStage())
                return;
            
            Draw(m_renderingInfo, targetCamera);
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
            this.transforms = transforms;
            reference = go;
            
            UpdateRenderingInfo(m_renderingInfo);
        }

        public void UpdateReference(GameObject go)
        {
            reference = go;
            UpdateRenderingInfo(m_renderingInfo);
        }

       
        private void UpdateRenderingInfo(InstanceRenderingInfo info)
        {
            // Transforms
            info.instances = transforms;
            
            // Reference
            info.gameObject = reference;
        }
        #endregion
        
        #region Rendering

        private void Draw(InstanceRenderingInfo info, Camera targetCamera)
        {
            if (targetCamera == null)
                return;
            
            if (!info.canRender)
                return;
            
            info.UpdateDividedInstances();
            
            DrawOnCamera(info, targetCamera);
        }
        
        private void Draw(InstanceRenderingInfo entry, Camera[] cameras)
        {
            if (!entry.canRender)
                return;
            
            entry.UpdateDividedInstances();

            if (cameras == null)
                return;
            
            for (var i = 0; i < cameras.Length; i++)
            {
                var targetCamera = cameras[i];
                if (targetCamera == null)
                    continue;
                
                DrawOnCamera(entry, targetCamera);
            }
        }

        private void DrawOnCamera(InstanceRenderingInfo entry, Camera targetCamera)
        {
            for (var submeshIndex = 0; submeshIndex < entry.mesh.subMeshCount; submeshIndex++)
            {
                // Try to get the material in the same index position as the mesh
                // or the last material.
                var materialIndex = Mathf.Clamp(submeshIndex, 0, entry.materials.Length - 1);

                var material = entry.materials[materialIndex];
                for (var matrixIndex = 0; matrixIndex < entry.dividedInstances.Count; matrixIndex++)
                {
                    var matrices = entry.dividedInstances[matrixIndex];

                    DrawOnCamera(
                        targetCamera,
                        entry.mesh,
                        submeshIndex,
                        material,
                        matrices,
                        entry.layer,
                        entry.receiveShadows,
                        entry.shadowCastingMode,
                        entry.lightProbeUsage,
                        entry.lightProbeProxyVolume);
                }
            }
        }

        private void DrawOnCamera(
            Camera targetCamera,
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
            if (targetCamera == null)
                return;
            
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
                camera:targetCamera,
                lightProbeUsage:lightProbeUsage, 
                lightProbeProxyVolume:lightProbeProxyVolume);
        }
        
        #endregion
    }

}

