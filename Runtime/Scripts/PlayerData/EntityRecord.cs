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
        
        Assert.IsNotNull(this.go);
        
        Light destLight = GetOrAddLight();

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
            
        Light destLight = GetOrAddLight();
        if (syncVisibility && this.hasVisibility)
            destLight.enabled = this.visibility.visibleInRender;
        destLight.type      = srcLight.type;
        destLight.color     = srcLight.color;
        destLight.intensity = srcLight.intensity;
        destLight.range     = srcLight.range;
        destLight.spotAngle = srcLight.spotAngle;

    }

    Light GetOrAddLight() {
        if (null != this.light) 
            return this.light;

        Assert.IsNotNull(this.go);
        this.light = Misc.GetOrAddComponent<Light>(this.go);
        return this.light;        
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
