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
        HashSet<VolumeProfile> volumeProfiles = new HashSet<VolumeProfile>();
        foreach (Volume v in volumes) {
            VolumeProfile profile = v.sharedProfile;
            volumeProfiles.Add(profile);
        }

        foreach (VolumeProfile vp in volumeProfiles) {
            if (vp.TryGet<PathTracing>(out PathTracing pathTracing)) {
                if (pathTracing.enable.value)
                    return true;
            }
        }
        
        return false;
    }
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

#if UNITY_2021_2_OR_NEWER
    internal static void ResetPathTracing() {
        HDRenderPipeline hdRenderPipeline = RenderPipelineManager.currentPipeline as HDRenderPipeline;
        hdRenderPipeline?.ResetPathTracing();
    }
#endif // UNITY_2021_2_OR_NEWER
}


} //end namespace

#endif //AT_USE_HDRP
