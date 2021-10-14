using System;
using UnityEngine;
using UnityEngine.Assertions;
using UnityEngine.Rendering.HighDefinition;

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
        Light          lt     = this.light;
        
        Assert.IsNotNull(this.go);
        if (lt == null)
            lt = this.light = Misc.GetOrAddComponent<Light>(this.go);

        if (syncVisibility && transformData.dataFlags.hasVisibility)
            lt.enabled = transformData.visibility.visibleInRender;

        LightType lightType = lightData.lightType;
        if ((int)lightType != -1)
            lt.type = lightData.lightType;
        if (flags.hasShadowType)
            lt.shadows = lightData.shadowType;

        if(flags.hasColor)
            lt.color = lightData.color;
        if (flags.hasIntensity)
            lt.intensity = lightData.intensity;
        if (flags.hasRange)
            lt.range = lightData.range;
        if (flags.hasSpotAngle)
            lt.spotAngle = lightData.spotAngle;
        return rec;

    }
    
    
//----------------------------------------------------------------------------------------------------------------------
    
    public EntityType          dataType;
    public int                 index;
    public GameObject          go;
    public Transform           trans;
    public Camera              camera;
    public Light               light;
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
