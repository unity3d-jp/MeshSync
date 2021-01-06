using JetBrains.Annotations;
using JetBrains.Annotations;

namespace Unity.AnimeToolbox.Editor {

internal static class PackageUtility {

    [CanBeNull]
    internal static PackageVersion ParseVersion(string ver) {
        string[] tokens = ver.Split('.');
        if (tokens.Length <= 2)
            return null;

        if (!int.TryParse(tokens[0], out int major))
            return null;

        if (!int.TryParse(tokens[1], out int minor))
            return null;

        //Find patch and lifecyle
        string[] patches = tokens[2].Split('-');
        if (!int.TryParse(patches[0], out int patch))
            return null;
        
        PackageLifecycle lifecycle = PackageLifecycle.INVALID;
        if (patches.Length > 1) {
            string           lifecycleStr = patches[1].ToLower();                    
            switch (lifecycleStr) {
                case "experimental": lifecycle = PackageLifecycle.EXPERIMENTAL; break;
                case "preview"   : lifecycle   = PackageLifecycle.EXPERIMENTAL; break;
                case "prerelease": lifecycle   = PackageLifecycle.PRERELEASE; break;
                default: lifecycle             = PackageLifecycle.INVALID; break;
            }
            
        } else {
            lifecycle = PackageLifecycle.RELEASED; 
            
        }
            

        PackageVersion packageVersion = new PackageVersion() {
            Major = major,
            Minor = minor,
            Patch = patch,
            Lifecycle = lifecycle
        };
        if (tokens.Length > 3) {
            packageVersion.AdditionalMetadata = tokens[3];
        }

        return packageVersion;

    } 

}

} //end namespace