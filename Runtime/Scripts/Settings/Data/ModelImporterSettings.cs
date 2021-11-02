using System;
using UnityEngine;

namespace Unity.MeshSync {

[Serializable]
internal class ModelImporterSettings {

    internal ModelImporterSettings() { }
    
    internal ModelImporterSettings(ModelImporterSettings other) {
        this.CreateMaterials = other.CreateMaterials;
        this.MaterialSearchMode = other.MaterialSearchMode;
    }
    
    internal ModelImporterSettings(bool createMaterials) {
        this.CreateMaterials = createMaterials;
    }
    

//----------------------------------------------------------------------------------------------------------------------
    
    [SerializeField] internal bool CreateMaterials;
    [SerializeField] internal AssetSearchMode MaterialSearchMode;

//----------------------------------------------------------------------------------------------------------------------
    
    //properties
    internal const string CREATE_MATERIALS_PROP     = "CreateMaterials";
    internal const string MATERIAL_SEARCH_MODE_PROP = "MaterialSearchMode";
    
    
}

} //end namespace