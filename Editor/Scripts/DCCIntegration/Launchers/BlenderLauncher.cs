using System;
using System.Diagnostics;
using System.IO;
using UnityEditor;
using UnityEngine;
using Debug = UnityEngine.Debug;

namespace Unity.MeshSync.Editor {
internal class BlenderLauncher : IDCCLauncher {
    public const string FileFormat = ".blend";

    private const string editorSettingPath = "MESHSYNC_BLENDER_PATH";

    private        Process m_blenderProcess;
    private static bool    redirectBlenderToUnityConsole;

    public bool    HasProcess => m_blenderProcess != null;
    
    public RunMode runMode    { get; set; }

    internal string InteropFile =
        Path.GetFullPath("Packages/com.unity.meshsync/Editor/Scripts/DCCIntegration/Launchers/blender_interop.py");

    private static string GetBlenderPath() {
        string blenderPath = EditorPrefs.GetString(editorSettingPath);

        if (string.IsNullOrEmpty(blenderPath)) {
            if (!DefaultAppUtility.TryGetRegisteredApplication(FileFormat, out blenderPath))
                if (Application.platform == RuntimePlatform.OSXEditor)
                    blenderPath = Path.Combine("Applications", "Blender.app", "Contents", "MacOS", "Blender");

            SetBlenderPath(blenderPath);
        }

        return blenderPath;
    }

    public static void SetBlenderPath(string blenderPath) {
        EditorPrefs.SetString(editorSettingPath, blenderPath);
    }

    private void M_ProcessOnOutputDataReceived(object sender, DataReceivedEventArgs e) {
        Debug.LogFormat("[BLENDER]\n{0}", e.Data);
    }

    private void M_ProcessOnErrorDataReceived(object sender, DataReceivedEventArgs e) {
        Debug.LogErrorFormat("[BLENDER]\n{0}", e.Data);
    }

    private void M_blenderProcess_Exited(object sender, EventArgs e) {
        m_blenderProcess = null;
    }

    #region IDCCLauncher
    
    public void OpenDCCTool(UnityEngine.Object asset)
    {
        string assetPath = AssetDatabase.GetAssetPath(asset);
        OpenDCCTool(assetPath);
    }

    public void OpenDCCTool(string assetPath)
    {
        assetPath = Path.GetFullPath(assetPath);
        
        Dispose();

        ProcessStartInfo startInfo = new ProcessStartInfo();
        startInfo.FileName = GetBlenderPath();

        if (string.IsNullOrEmpty(startInfo.FileName) || !File.Exists(startInfo.FileName)) {
            Debug.LogError("The blender path is not set or blender cannot be found.");
            return;
        }

        string absoluteAssetPath = Path.Combine(Application.dataPath, assetPath).Replace('\\', '/');

        startInfo.Arguments = $"\"{absoluteAssetPath}\" -P \"{InteropFile}\"";

        if (runMode != RunMode.GUI)
            startInfo.Arguments = "-b " + startInfo.Arguments;

        if (runMode == RunMode.Background) {
            startInfo.UseShellExecute = false;
            startInfo.CreateNoWindow  = true;
            startInfo.WindowStyle     = ProcessWindowStyle.Hidden;
        }

        m_blenderProcess = new Process();

        if (redirectBlenderToUnityConsole) {
            startInfo.RedirectStandardOutput = true;
            startInfo.RedirectStandardError  = true;

            m_blenderProcess.ErrorDataReceived  -= M_ProcessOnErrorDataReceived;
            m_blenderProcess.OutputDataReceived -= M_ProcessOnOutputDataReceived;

            m_blenderProcess.ErrorDataReceived  += M_ProcessOnErrorDataReceived;
            m_blenderProcess.OutputDataReceived += M_ProcessOnOutputDataReceived;

            startInfo.UseShellExecute = false;
        }

        m_blenderProcess.StartInfo = startInfo;

        m_blenderProcess.EnableRaisingEvents =  true;
        m_blenderProcess.Exited              += M_blenderProcess_Exited;

        m_blenderProcess.Start();

        if (redirectBlenderToUnityConsole) {
            m_blenderProcess.BeginOutputReadLine();
            m_blenderProcess.BeginErrorReadLine();
        }
    }

    public void CloseDCCTool() {
        try {
            m_blenderProcess.Kill();
        }
        catch {
            // Exceptions don't matter here, just close it no matter what.
        }

        m_blenderProcess = null;
    }

    private bool HandleBlenderPath(MeshSyncServer server) {
        bool changed = false;

        string blenderPath = GetBlenderPath();

        GUILayout.BeginHorizontal();

        GUIStyle style = new GUIStyle(GUI.skin.label) { wordWrap = true };
        EditorGUILayout.LabelField("Blender path:", blenderPath, style);
        if (GUILayout.Button("...", GUILayout.Width(30))) {
            string newBlenderPath = EditorUtility.OpenFilePanel("Blender path", Path.GetDirectoryName(blenderPath), "exe");

            if (!string.IsNullOrEmpty(newBlenderPath) && newBlenderPath != blenderPath) {
                SetBlenderPath(newBlenderPath);
                OpenDCCTool(server.DCCAsset);
            }

            changed = true;
        }

        GUILayout.EndHorizontal();

        return changed;
    }

    private void HandleRunMmode(MeshSyncServer server) {
        GUILayout.BeginHorizontal();

        RunMode newRunMode = (RunMode)EditorGUILayout.EnumPopup("Run mode:", runMode);
        if (newRunMode != runMode) {
            runMode = newRunMode;
            if (m_blenderProcess != null) OpenDCCTool(server.DCCAsset);
        }

        GUILayout.EndHorizontal();
    }

    private void HandleRedirect(MeshSyncServer server) {
        bool newRedirect = EditorGUILayout.Toggle("Redirect DCC Tool to Unity Console:", redirectBlenderToUnityConsole);
        if (newRedirect != redirectBlenderToUnityConsole) {
            redirectBlenderToUnityConsole = newRedirect;
            if (m_blenderProcess != null) OpenDCCTool(server.DCCAsset);
        }
    }

    public void DrawDCCMenu(BaseMeshSync player) {
        MeshSyncServer server = player as MeshSyncServer;

        if (server == null) return;

        GUILayout.BeginVertical("Box");

        if (HandleBlenderPath(server)) {
            GUILayout.EndVertical();
            return;
        }

        HandleRunMmode(server);

        HandleRedirect(server);

        GUILayout.EndVertical();

        EditorGUILayout.Space();
    }

    public void Dispose() {
        if (m_blenderProcess == null) return;

        // Only force close blender process if it was running in
        // background mode and the user has no way to close it
        // themselves and have no pending changes:
        try {
            if (m_blenderProcess.StartInfo.WindowStyle == ProcessWindowStyle.Hidden)
                if (!m_blenderProcess.HasExited)
                    m_blenderProcess.Kill();
        }
        catch {
        }

        m_blenderProcess = null;
    }

    #endregion // IDCCLauncher
}
}