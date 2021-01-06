namespace Unity.AnimeToolbox.Editor {

internal class PackageVersion {
    public PackageVersion() {
        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    public PackageVersion(string semanticVer) {
        if (!PackageUtility.TryParseVersion(semanticVer, out PackageVersion temp))
            return;
        
        this.Major     = temp.Major;
        this.Minor     = temp.Minor;
        this.Patch     = temp.Patch;
        this.Lifecycle = temp.Lifecycle;
        this.AdditionalMetadata = temp.AdditionalMetadata;
    }
    
//----------------------------------------------------------------------------------------------------------------------
    public int              Major;
    public int              Minor;
    public int              Patch;
    public PackageLifecycle Lifecycle;
    public string           AdditionalMetadata;
    
    
    

}

}

