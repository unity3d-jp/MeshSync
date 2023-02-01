using System;
using UnityEngine;

namespace Unity.MeshSync {
[Serializable]
internal class ModelImporterSettings {
    internal ModelImporterSettings() {
    }

    internal ModelImporterSettings(ModelImporterSettings other) {
        CreateMaterials    = other.CreateMaterials;
        MaterialSearchMode = other.MaterialSearchMode;
    }

//----------------------------------------------------------------------------------------------------------------------

    [SerializeField] internal bool            CreateMaterials            = true;
    [SerializeField] internal AssetSearchMode MaterialSearchMode         = AssetSearchMode.LOCAL;
    [SerializeField] internal bool            OverwriteExportedMaterials = false;

//----------------------------------------------------------------------------------------------------------------------

    //properties
    internal const string CREATE_MATERIALS_PROP        = "CreateMaterials";
    internal const string MATERIAL_SEARCH_MODE_PROP    = "MaterialSearchMode";
    internal const string OVERWRITE_EXPORTED_MATERIALS = "OverwriteExportedMaterials";
}
} //end namespace