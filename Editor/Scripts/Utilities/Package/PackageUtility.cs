using JetBrains.Annotations;

namespace Unity.AnimeToolbox.Editor {

internal static class PackageUtility {

    /// <summary>
    /// Parse a semantic versioned string to a PackageVersion class
    /// </summary>
    /// <param name="semanticVer">Semantic versioned input string</param>
    /// <param name="packageVersion">The detected PackageVersion. Set to null when the parsing fails</param>
    /// <returns>true if successful, false otherwise</returns>
    internal static bool TryParseVersion(string semanticVer, out PackageVersion packageVersion) {
        packageVersion = null;
        string[] tokens = semanticVer.Split('.');
        if (tokens.Length <= 2)
            return false;

        if (!int.TryParse(tokens[0], out int major))
            return false;

        if (!int.TryParse(tokens[1], out int minor))
            return false;

        //Find patch and lifecyle
        string[] patches = tokens[2].Split('-');
        if (!int.TryParse(patches[0], out int patch))
            return false;
        
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

        packageVersion = new PackageVersion() {
            Major = major,
            Minor = minor,
            Patch = patch,
            Lifecycle = lifecycle
        };
        if (tokens.Length > 3) {
            packageVersion.AdditionalMetadata = tokens[3];
        }

        return true;

    } 

//----------------------------------------------------------------------------------------------------------------------



}

} //end namespace