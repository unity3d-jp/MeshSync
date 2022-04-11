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
        enum RunMode
        {
            GUI,
            Background,
            Console
        }

        RunMode runMode { get; set; }

        void OpenDCCTool(UnityEngine.Object asset);

        void DrawDCCToolVersion(BaseMeshSync player);

        void Cleanup();
    }
}
