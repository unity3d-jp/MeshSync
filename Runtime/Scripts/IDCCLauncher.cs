using System;

namespace Unity.MeshSync
{
    internal enum RunMode
    {
        GUI,
        Background,
        Console
    }

    /// <summary>
    /// Interface for classes that handle DCC tool control from inside Unity.
    /// </summary>
    internal interface IDCCLauncher : IDisposable
    {
        RunMode runMode { get; set; }

        void OpenDCCTool(UnityEngine.Object asset);

        void CloseDCCTool();

        void DrawDCCMenu(BaseMeshSync player);
    }
}
