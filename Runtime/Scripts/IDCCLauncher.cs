using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

namespace Unity.MeshSync
{
    /// <summary>
    /// Interface for classes that handle DCC tool control from inside Unity.
    /// </summary>
    public interface IDCCLauncher
    {
        void OpenDCCTool(GameObject asset);

        void DrawDCCToolVersion(BaseMeshSync player);

        void Cleanup();
    }
}
