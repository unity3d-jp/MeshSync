using System;
using System.Collections.Generic;

namespace Unity.MeshSync.Editor {

public class BlenderVersionVerifier : IDccToolVersionVerifier {
    public class VersionRange{

        public VersionRange(int first, int last) {
            First = first;
            Last  = last;
        }
        
        public int First { get; set; }
        public int Last  { get; set; }

        public bool InRange(int version) {
            return version >= First && version <= Last;
        }
    }
    
    private Dictionary<int, VersionRange> supportedVersions = new Dictionary<int, VersionRange> {
        {2, new VersionRange(83, 93)}, //2.70 to 2.94
        {3, new VersionRange(0, 1)}, //3.0 to 3.1
    };
    
    public bool IsSupported(string version) {
        
        var versionSequence = version.Split('.');
        
        if (versionSequence.Length < 2)
            throw new ArgumentOutOfRangeException($"[MeshSync]: Version of Blender: {version} " +
                $"should follow this format: <blender>.<major>.<minor>");
        
        var blender = int.Parse(versionSequence[0]);
        var major   = int.Parse(versionSequence[1]);

        if (!supportedVersions.TryGetValue(blender, out VersionRange range)) {
            return false;
        }

        return range.InRange(major);
    }

    public string LatestSupportedVersion => "3.1";
}
}