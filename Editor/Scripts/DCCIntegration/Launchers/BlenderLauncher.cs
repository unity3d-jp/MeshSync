using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor
{
    internal class BlenderLauncher : IDCCLauncher
    {
        const string menuItem_OpenInBlender = "Assets/MeshSync/Open in blender";

        const string editorSettingPath = "MESHSYNC_BLENDER_PATH";

        System.Diagnostics.Process m_blenderProcess;
        static bool redirectBlenderToUnityConsole;

        public RunMode runMode { get; set; }

        ~BlenderLauncher()
        {
            Cleanup();
        }

        static string GetBlenderPath()
        {
            var blenderPath = EditorPrefs.GetString(editorSettingPath);

            if (string.IsNullOrEmpty(blenderPath))
            {
                if (!DefaultAppUtility.TryGetRegisteredApplication(".blend", out blenderPath))
                {
                    if (Application.platform == RuntimePlatform.OSXEditor)
                    {
                        blenderPath = "/Applications/Blender.app/Contents/MacOS/Blender";
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

        public static void OpenBlendFile(MeshSyncServer server, UnityEngine.Object asset)
        {
            var previousRunMode = server.m_DCCInterop != null ? server.m_DCCInterop.runMode : RunMode.GUI;

            server.m_DCCInterop?.Cleanup();
            server.m_DCCInterop = MeshSyncServerPropertiesInspector.GetLauncherForAsset(asset);
            server.m_DCCInterop.runMode = previousRunMode;
            server.m_DCCInterop.OpenDCCTool(asset);
        }

        [MenuItem(menuItem_OpenInBlender, priority = 0)]
        static void OpenBlender()
        {
            var selectedAsset = Selection.objects[0];
            if (selectedAsset == null)
                return;

            var servers = UnityEngine.Object.FindObjectsOfType<MeshSyncServer>();

            MeshSyncServer server = null;

            foreach (var serverInScene in servers)
            {
                if (serverInScene.enabled)
                {
                    server = serverInScene;
                    break;
                }
            }

            if (server == null)
            {
                server = MeshSyncMenu.CreateMeshSyncServerMenu(null);
            }

            server.m_DCCAsset = selectedAsset;

            OpenBlendFile(server, selectedAsset);
        }

        [MenuItem(menuItem_OpenInBlender, validate = true, priority = 0)]
        static bool CanOpenBlender()
        {
            if (Selection.objects.Length == 1)
            {
                string assetPath = AssetDatabase.GetAssetPath(Selection.objects[0]);
                if (assetPath.EndsWith(".blend"))
                {
                    return true;
                }
            }
            return false;
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
            Cleanup();

            var startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.FileName = GetBlenderPath();

            var assetPath = AssetDatabase.GetAssetPath(asset).Replace("Assets/", string.Empty);
            var absoluteAssetPath = Path.Combine(Application.dataPath, assetPath).Replace('\\', '/');

            var scriptPath = Path.GetFullPath("Packages/com.unity.meshsync/Editor/Scripts/DCCIntegration/Launchers/blender_interop.py");

            startInfo.Arguments = $"\"{absoluteAssetPath}\" -P \"{scriptPath}\"";

            //startInfo.Arguments = $"-P \"{s_ScriptPath}\" -- {port}";
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
            //m_blenderProcess.Start();
            m_blenderProcess.EnableRaisingEvents = true;
            m_blenderProcess.Exited += M_blenderProcess_Exited;

            m_blenderProcess.Start();

            if (redirectBlenderToUnityConsole)
            {
                m_blenderProcess.BeginOutputReadLine();
                m_blenderProcess.BeginErrorReadLine();
            }
        }

        public void DrawDCCToolVersion(BaseMeshSync player)
        {
            GUILayout.BeginVertical("Box");

            if (player is MeshSyncServer server)
            {
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
                        OpenBlendFile(server, server.m_DCCAsset);
                    }
                    return;
                }

                GUILayout.EndHorizontal();

                GUILayout.BeginHorizontal();

                var newRunMode = (RunMode)EditorGUILayout.EnumPopup("Run mode:", runMode);
                if (newRunMode != runMode)
                {
                    runMode = newRunMode;
                    if (m_blenderProcess != null)
                    {
                        OpenDCCTool(server.m_DCCAsset);
                    }
                }

                GUILayout.EndHorizontal();

                var newRedirect = EditorGUILayout.Toggle("Redirect Blender to Unity Console:", redirectBlenderToUnityConsole);
                if (newRedirect != redirectBlenderToUnityConsole)
                {
                    redirectBlenderToUnityConsole = newRedirect;
                    if (m_blenderProcess != null)
                    {
                        OpenDCCTool(server.m_DCCAsset);
                    }
                }
            }

            GUILayout.EndVertical();

            EditorGUILayout.Space();
        }

        public void Cleanup()
        {
            if (m_blenderProcess != null)
            {
                try
                {
                    if (!m_blenderProcess.HasExited)
                    {
                        m_blenderProcess.Kill();
                    }
                }
                catch { }

                m_blenderProcess = null;
            }
        }

        #endregion // IDCCLauncher
    }
}

