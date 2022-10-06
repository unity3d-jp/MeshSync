#if AT_USE_HDRP

using System.Collections.Generic;
using UnityEngine.Rendering;
using UnityEngine.Rendering.HighDefinition;


namespace Unity.MeshSync {

internal static class HDRPUtility  {

    internal static bool IsRayTracingActive() {
        HDRenderPipelineAsset hdRenderPipelineAsset = GraphicsSettings.renderPipelineAsset as HDRenderPipelineAsset;
        if (null == hdRenderPipelineAsset)
            return false;

        return hdRenderPipelineAsset.currentPlatformRenderPipelineSettings.supportRayTracing;
    }
    
    internal static bool IsPathTracingActive(IEnumerable<Volume> volumes) {
        foreach (Volume v in volumes) {
            VolumeProfile volumeProfile = v.sharedProfile;
            if (volumeProfile.TryGet<PathTracing>(out PathTracing pathTracing)) {
                if (pathTracing.enable.value)
                    return true;
            }
        }
        return false;
    }
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    internal static void ResetPathTracing() {
        HDRenderPipeline hdRenderPipeline = RenderPipelineManager.currentPipeline as HDRenderPipeline;
        hdRenderPipeline?.ResetPathTracing();
    }
    
}


} //end namespace

#endif //AT_USE_HDRP
