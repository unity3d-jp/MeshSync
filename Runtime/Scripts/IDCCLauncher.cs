namespace Unity.MeshSync
{
    public enum RunMode
    {
        GUI,
        Background,
        Console
    }

    /// <summary>
    /// Interface for classes that handle DCC tool control from inside Unity.
    /// </summary>
    public interface IDCCLauncher
    {
        RunMode runMode { get; set; }

        void OpenDCCTool(UnityEngine.Object asset);

        void DrawDCCToolVersion(BaseMeshSync player);

        void Cleanup();
    }
}
