
using System.Collections.Generic;
using UnityEngine.Rendering;
using UnityEngine.Rendering.HighDefinition;

#if AT_USE_HDRP

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
            if (v.TryGetComponent(out PathTracing pathTracing)) {
                if (pathTracing.active)
                    return true;
            }
        }
        return false;
    }
}


} //end namespace

#endif //AT_USE_HDRP
