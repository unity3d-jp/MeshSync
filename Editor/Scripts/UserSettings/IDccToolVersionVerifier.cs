using UnityEngine;

namespace Unity.MeshSync.Editor {
public interface IDccToolVersionVerifier {
    bool   IsSupported(string version);
    string LatestSupportedVersion{ get;}
}
}