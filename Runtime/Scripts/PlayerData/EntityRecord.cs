using System;
using UnityEngine;
using UnityEngine.Assertions;

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
               
        LightRecordComponents components = GetOrAddLight();

        if (syncVisibility && transformData.dataFlags.hasVisibility)
            components.LightComponent.enabled = transformData.visibility.visibleInRender;

        LightType lightType = lightData.lightType;
        if ((int)lightType != -1)
            components.LightComponent.type = lightData.lightType;
        if (flags.hasShadowType)
            components.LightComponent.shadows = lightData.shadowType;

        if(flags.hasColor)
            components.LightComponent.color = lightData.color;
        if (flags.hasIntensity)
            components.LightComponent.intensity = lightData.intensity;
        if (flags.hasRange)
            components.LightComponent.range = lightData.range;
        if (flags.hasSpotAngle)
            components.LightComponent.spotAngle = lightData.spotAngle;

    }
    
    internal void SetLight(EntityRecord srcRecord, bool syncVisibility) {
        
        Light srcLight = srcRecord.light;
        if (null == srcLight) 
            return;
            
        LightRecordComponents components = GetOrAddLight();
        if (syncVisibility && this.hasVisibility)
            components.LightComponent.enabled = this.visibility.visibleInRender;
        components.LightComponent.type      = srcLight.type;
        components.LightComponent.color     = srcLight.color;
        components.LightComponent.intensity = srcLight.intensity;
        components.LightComponent.range     = srcLight.range;
        components.LightComponent.spotAngle = srcLight.spotAngle;

    }

    LightRecordComponents GetOrAddLight() {

        LightRecordComponents recComponents;
        if (null == this.light) {
            Assert.IsNotNull(this.go);
            this.light = Misc.GetOrAddComponent<Light>(this.go);            
        } 

        Assert.IsNotNull(this.light);
        recComponents.LightComponent = this.light;
        return recComponents;        
    }

//----------------------------------------------------------------------------------------------------------------------

    private struct LightRecordComponents {
        internal Light LightComponent;
    }    
    
//----------------------------------------------------------------------------------------------------------------------
    
    public EntityType          dataType;
    public int                 index;
    public GameObject          go;
    public Transform           trans;
    public Camera              camera;
    
    [SerializeField] private Light               light;
    
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
