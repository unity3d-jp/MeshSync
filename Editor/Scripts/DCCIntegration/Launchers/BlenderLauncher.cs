using System;
using System.Diagnostics;
using System.IO;
using UnityEditor;
using UnityEngine;
using Debug = UnityEngine.Debug;

namespace Unity.MeshSync.Editor
{
    internal class BlenderLauncher : IDCCLauncher
    {
        public const string FileFormat = ".blend";

        const string editorSettingPath = "MESHSYNC_BLENDER_PATH";

        Process m_blenderProcess;
        static bool redirectBlenderToUnityConsole;

        public RunMode runMode { get; set; }
        
        static string GetBlenderPath()
        {
            var blenderPath = EditorPrefs.GetString(editorSettingPath);

            if (string.IsNullOrEmpty(blenderPath))
            {
                if (!DefaultAppUtility.TryGetRegisteredApplication(BlenderLauncher.FileFormat, out blenderPath))
                {
                    if (Application.platform == RuntimePlatform.OSXEditor)
                    {
                        blenderPath = Path.Combine("Applications", "Blender.app", "Contents", "MacOS", "Blender");
                    }
                }

                SetBlenderPath(blenderPath);
            }

            return blenderPath;
        }

        public static void SetBlenderPath(string blenderPath)
        {
            EditorPrefs.SetString(editorSettingPath, blenderPath);
        }

        private void M_ProcessOnOutputDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            Debug.LogFormat("[BLENDER]\n{0}", e.Data);
        }

        private void M_ProcessOnErrorDataReceived(object sender, System.Diagnostics.DataReceivedEventArgs e)
        {
            Debug.LogErrorFormat("[BLENDER]\n{0}", e.Data);
        }

        private void M_blenderProcess_Exited(object sender, EventArgs e)
        {
            m_blenderProcess = null;
        }

        #region IDCCLauncher

        public void OpenDCCTool(UnityEngine.Object asset)
        {
            Dispose();

            var startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.FileName = GetBlenderPath();

            if (string.IsNullOrEmpty(startInfo.FileName) || !File.Exists(startInfo.FileName))
            {
                Debug.LogError("The blender path is not set or blender cannot be found.");
                return;
            }

            var assetPath = FilmInternalUtilities.AssetUtility.ToAssetRelativePath(AssetDatabase.GetAssetPath(asset));

            var absoluteAssetPath = Path.Combine(Application.dataPath, assetPath).Replace('\\', '/');

            var scriptPath = Path.GetFullPath("Packages/com.unity.meshsync/Editor/Scripts/DCCIntegration/Launchers/blender_interop.py");

            startInfo.Arguments = $"\"{absoluteAssetPath}\" -P \"{scriptPath}\"";

            if (runMode != RunMode.GUI)
                startInfo.Arguments = "-b " + startInfo.Arguments;

            if (runMode == RunMode.Background)
            {
                startInfo.UseShellExecute = false;
                startInfo.CreateNoWindow = true;
                startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            }

            m_blenderProcess = new System.Diagnostics.Process();

            if (redirectBlenderToUnityConsole)
            {
                startInfo.RedirectStandardOutput = true;
                startInfo.RedirectStandardError = true;

                m_blenderProcess.ErrorDataReceived -= M_ProcessOnErrorDataReceived;
                m_blenderProcess.OutputDataReceived -= M_ProcessOnOutputDataReceived;

                m_blenderProcess.ErrorDataReceived += M_ProcessOnErrorDataReceived;
                m_blenderProcess.OutputDataReceived += M_ProcessOnOutputDataReceived;

                startInfo.UseShellExecute = false;
            }

            m_blenderProcess.StartInfo = startInfo;

            m_blenderProcess.EnableRaisingEvents = true;
            m_blenderProcess.Exited += M_blenderProcess_Exited;

            m_blenderProcess.Start();

            if (redirectBlenderToUnityConsole)
            {
                m_blenderProcess.BeginOutputReadLine();
                m_blenderProcess.BeginErrorReadLine();
            }
        }

        bool HandleBlenderPath(MeshSyncServer server)
        {
            bool changed = false;

            var blenderPath = GetBlenderPath();

            GUILayout.BeginHorizontal();

            var style = new GUIStyle(GUI.skin.label) { wordWrap = true };
            EditorGUILayout.LabelField("Blender path:", blenderPath, style);
            if (GUILayout.Button("...", GUILayout.Width(30)))
            {
                var newBlenderPath = EditorUtility.OpenFilePanel("Blender path", Path.GetDirectoryName(blenderPath), "exe");

                if (!string.IsNullOrEmpty(newBlenderPath) && newBlenderPath != blenderPath)
                {
                    SetBlenderPath(newBlenderPath);
                    OpenDCCTool(server.DCCAsset);
                }

                changed = true;
            }

            GUILayout.EndHorizontal();

            return changed;
        }

        void HandleRunMmode(MeshSyncServer server)
        {
            GUILayout.BeginHorizontal();

            var newRunMode = (RunMode)EditorGUILayout.EnumPopup("Run mode:", runMode);
            if (newRunMode != runMode)
            {
                runMode = newRunMode;
                if (m_blenderProcess != null)
                {
                    OpenDCCTool(server.DCCAsset);
                }
            }

            GUILayout.EndHorizontal();
        }

        void HandleRedirect(MeshSyncServer server)
        {
            var newRedirect = EditorGUILayout.Toggle("Redirect Blender to Unity Console:", redirectBlenderToUnityConsole);
            if (newRedirect != redirectBlenderToUnityConsole)
            {
                redirectBlenderToUnityConsole = newRedirect;
                if (m_blenderProcess != null)
                {
                    OpenDCCTool(server.DCCAsset);
                }
            }
        }

        public void DrawDCCMenu(BaseMeshSync player)
        {
            var server = player as MeshSyncServer;

            if (server == null)
            {
                return;
            }

            GUILayout.BeginVertical("Box");

            if (HandleBlenderPath(server))
            {
                GUILayout.EndVertical();
                return;
            }

            HandleRunMmode(server);

            HandleRedirect(server);

            GUILayout.EndVertical();

            EditorGUILayout.Space();
        }

        public void Dispose()
        {
            if (m_blenderProcess == null)
            {
                return;
            }
            
            // Only force close blender process if it was running in
            // background mode and the user has no way to close it
            // themselves and have no pending changes:
            try
            {
                if (m_blenderProcess.StartInfo.WindowStyle == ProcessWindowStyle.Hidden)
                {
                    if (!m_blenderProcess.HasExited)
                    {
                        m_blenderProcess.Kill();
                    }
                }
            }
            catch { }

            m_blenderProcess = null;
        }

        #endregion // IDCCLauncher
    }
}

