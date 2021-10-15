using System;
using UnityEngine;
using UnityEngine.Assertions;

#if AT_USE_HDRP
using UnityEngine.Rendering.HighDefinition;
#endif
    
namespace Unity.MeshSync {

[Serializable]
internal class EntityRecord {

    // return true if modified
    public bool BuildMaterialData(MeshData md) {
        int numSubmeshes = md.numSubmeshes;

        bool updated = false;
        if (materialIDs == null || materialIDs.Length != numSubmeshes) {
            materialIDs = new int[numSubmeshes];
            updated = true;
        }
        
        for (int i = 0; i < numSubmeshes; ++i) {
            int mid = md.GetSubmesh(i).materialID;
            if (!updated)
                updated = materialIDs[i] != mid;
            materialIDs[i] = mid;
        }
        return updated;
    }
//----------------------------------------------------------------------------------------------------------------------

    internal void SetLight(LightData lightData, bool syncVisibility) {
        TransformData  transformData = lightData.transform;
        LightDataFlags flags = lightData.dataFlags;
               
        LightComponents components = GetOrAddLight();
        Light destLight  = components.LightComponent;

        if (syncVisibility && transformData.dataFlags.hasVisibility)
            destLight.enabled = transformData.visibility.visibleInRender;

        LightType lightType = lightData.lightType;
        if ((int)lightType != -1)
            destLight.type = lightData.lightType;
        if (flags.hasShadowType)
            destLight.shadows = lightData.shadowType;

        if(flags.hasColor)
            destLight.color = lightData.color;
        if (flags.hasIntensity)
            destLight.intensity = lightData.intensity;
        if (flags.hasRange)
            destLight.range = lightData.range;
        if (flags.hasSpotAngle)
            destLight.spotAngle = lightData.spotAngle;

    }
    
    internal void SetLight(EntityRecord srcRecord, bool syncVisibility) {
        
        Light srcLight = srcRecord.light;
        if (null == srcLight) 
            return;
            
        LightComponents components = GetOrAddLight();
        Light destLight  = components.LightComponent;
        
        if (syncVisibility && this.hasVisibility)
            destLight.enabled = this.visibility.visibleInRender;
        destLight.type      = srcLight.type;
        destLight.color     = srcLight.color;
        destLight.intensity = srcLight.intensity;
        destLight.range     = srcLight.range;
        destLight.spotAngle = srcLight.spotAngle;
        
#if AT_USE_HDRP
        HDAdditionalLightData destHDLightData = components.HDAdditionalLightDataComponent;
        destHDLightData.SetTypeFromLegacy(srcLight.type);
        destHDLightData.color     = srcLight.color;
        destHDLightData.intensity = srcLight.intensity;
        destHDLightData.range     = srcLight.range;
        destHDLightData.SetSpotAngle(srcLight.spotAngle);
#endif


    }

    LightComponents GetOrAddLight() {

        LightComponents recComponents;
        Assert.IsNotNull(this.go);
        if (null == this.light) {
            this.light = Misc.GetOrAddComponent<Light>(this.go);
        }
        Assert.IsNotNull(this.light);
        recComponents.LightComponent = this.light;

#if AT_USE_HDRP    
        if (null == m_hdAdditionalLightData) {
            m_hdAdditionalLightData = Misc.GetOrAddComponent<HDAdditionalLightData>(this.go);
        }
        Assert.IsNotNull(m_hdAdditionalLightData);
        recComponents.HDAdditionalLightDataComponent = m_hdAdditionalLightData;
#endif
        
        return recComponents;        
    }

//----------------------------------------------------------------------------------------------------------------------

    private struct LightComponents {
        internal Light LightComponent;
        
#if AT_USE_HDRP    
        internal HDAdditionalLightData HDAdditionalLightDataComponent;
#endif    
    }    
    
//----------------------------------------------------------------------------------------------------------------------
    
    public EntityType          dataType;
    public int                 index;
    public GameObject          go;
    public Transform           trans;
    public Camera              camera;
    
    [SerializeField] private Light               light;
#if AT_USE_HDRP    
    [SerializeField] private HDAdditionalLightData m_hdAdditionalLightData;
#endif    
    public MeshFilter          meshFilter;
    public MeshRenderer        meshRenderer;
    public SkinnedMeshRenderer skinnedMeshRenderer;
    public PointCache          pointCache;
    public PointCacheRenderer  pointCacheRenderer;
    public Mesh                origMesh;
    public Mesh                mesh;

    public int[]           materialIDs;
    public string          reference;
    public string          rootBonePath;
    public string[]        bonePaths;
    public bool            smrUpdated    = false;
    public bool            smrEnabled    = false;
    public bool            hasVisibility = false;
    public VisibilityFlags visibility; // for reference
    public bool            recved = false;
    
}


} //end namespace
