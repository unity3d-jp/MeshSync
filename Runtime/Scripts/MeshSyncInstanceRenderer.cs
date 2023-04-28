#if UNITY_EDITOR
#if !UNITY_2021_2_OR_NEWER
using UnityEditor.Experimental.SceneManagement;
#endif
#endif
using UnityEngine;
using UnityEngine.Rendering;

namespace Unity.MeshSync {
[ExecuteInEditMode]
internal class MeshSyncInstanceRenderer : MonoBehaviour {
    private readonly InstanceRenderingInfo m_renderingInfo = new InstanceRenderingInfo();

    [HideInInspector] [SerializeField] private Matrix4x4[] transforms;
    [SerializeField]                   private GameObject  reference;

    public int TransformCount {
        get { return transforms.Length; }
    }

    private void OnEnable() {
        if (GraphicsSettings.currentRenderPipeline == null)
            Camera.onPreCull += OnCameraPreCull;
        else
            RenderPipelineManager.beginFrameRendering += OnBeginFrameRendering;

        UpdateRenderingInfo(m_renderingInfo);
    }

    private void OnDisable() {
        if (GraphicsSettings.currentRenderPipeline == null)
            Camera.onPreCull -= OnCameraPreCull;
        else
            RenderPipelineManager.beginFrameRendering -= OnBeginFrameRendering;
    }

    private void OnBeginFrameRendering(ScriptableRenderContext arg1, Camera[] cameras) {
        if (!IsInPrefabStage())
            return;

        Draw(m_renderingInfo, cameras);
    }

    private void OnCameraPreCull(Camera targetCamera) {
        if (targetCamera.cameraType == CameraType.Preview)
            return;

        if (!IsInPrefabStage())
            return;

        Draw(m_renderingInfo, targetCamera);
    }


    private bool IsInPrefabStage() {
#if UNITY_EDITOR

#if UNITY_2021_2_OR_NEWER
            var stage = UnityEditor.SceneManagement.PrefabStageUtility.GetCurrentPrefabStage();
#else
        PrefabStage stage = PrefabStageUtility.GetCurrentPrefabStage();
#endif
        return stage == null || stage.IsPartOfPrefabContents(gameObject);
#else
            return true;
#endif
    }

    #region Events

    public void UpdateAll(Matrix4x4[] transforms, GameObject go) {
        this.transforms = transforms;
        reference       = go;

        UpdateRenderingInfo(m_renderingInfo);
    }

    public void UpdateReference(GameObject go) {
        reference = go;

        UpdateRenderingInfo(m_renderingInfo);
    }

    public void UpdateMaterials()
    {
        m_renderingInfo.UpdateMaterials();
    }


    private void UpdateRenderingInfo(InstanceRenderingInfo info) {
        info.instances      = transforms;
        info.gameObject     = reference;
        info.instanceParent = transform;
    }

    #endregion

    #region Rendering

    private void Draw(InstanceRenderingInfo info, Camera targetCamera) {
        if (targetCamera == null)
            return;

        if (!info.canRender)
            return;

        info.PrepareForDrawing();

        DrawOnCamera(info, targetCamera);
    }

    private void Draw(InstanceRenderingInfo info, Camera[] cameras) {
        if (!info.canRender)
            return;

        info.PrepareForDrawing();

        if (cameras == null)
            return;

        for (int i = 0; i < cameras.Length; i++) {
            Camera targetCamera = cameras[i];
            if (targetCamera == null)
                continue;

            DrawOnCamera(info, targetCamera);
        }
    }

    private void DrawOnCamera(InstanceRenderingInfo entry, Camera targetCamera) {
        for (int submeshIndex = 0; submeshIndex < entry.mesh.subMeshCount; submeshIndex++) {
            // Try to get the material in the same index position as the mesh
            // or the last material.
            int materialIndex = Mathf.Clamp(submeshIndex, 0, entry.materials.Length - 1);

            Material material = entry.materials[materialIndex];
            for (int batchIndex = 0; batchIndex < entry.batches.Count; batchIndex++) {
                InstancesBatch        batch      = entry.batches[batchIndex];
                Matrix4x4[]           matrices   = batch.Matrices;
                MaterialPropertyBlock properties = batch.PropertyBlock;

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
                    entry.lightProbeProxyVolume,
                    properties);
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
        LightProbeProxyVolume lightProbeProxyVolume,
        MaterialPropertyBlock properties) {
        if (targetCamera == null)
            return;

        Graphics.DrawMeshInstanced(
            mesh,
            submeshIndex,
            material,
            matrices,
            matrices.Length,
            properties,
            shadowCastingMode,
            receiveShadows,
            layer,
            targetCamera,
            lightProbeUsage,
            lightProbeProxyVolume);
    }

    #endregion
}
}