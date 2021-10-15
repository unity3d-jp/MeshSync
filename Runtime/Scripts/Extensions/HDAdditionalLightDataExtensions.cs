
#if AT_USE_HDRP

using UnityEngine;
using UnityEngine.Rendering.HighDefinition;

namespace Unity.MeshSync {

internal static class HDAdditionalLightDataExtensions {

    internal static void SetTypeFromLegacy(this HDAdditionalLightData lightData, LightType legacyLightType) {

        switch (legacyLightType) {
            case LightType.Directional: {
                lightData.type = HDLightType.Directional;
                break;
            }
            case LightType.Spot: {
                lightData.type = HDLightType.Spot;
                break;
            }            
            case LightType.Point: {
                lightData.type = HDLightType.Point;
                break;
            }
            case LightType.Disc: {
                lightData.type           = HDLightType.Area;
                lightData.areaLightShape = AreaLightShape.Disc;
                break;
            }
            case LightType.Area: {
                lightData.type           = HDLightType.Area;
                lightData.areaLightShape = AreaLightShape.Rectangle;
                break;
            }
            default: {
                Debug.LogWarning($"[MeshSync] Unsupported legacy light type: {legacyLightType}");
                break;
            };
        
        }        
    }
    

}

} //end namespace

#endif //#if AT_USE_HDRP    
    
