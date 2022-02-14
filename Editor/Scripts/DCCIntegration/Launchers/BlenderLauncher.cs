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
        enum RunMode
        {
            Background,
            Console,
            GUI
        }

        const string menuItem_OpenInBlender = "Assets/MeshSync/Open in blender";

        const string editorSettingPath = "MESHSYNC_BLENDER_PATH";

        System.Diagnostics.Process m_blenderProcess;
        bool m_redirectBlenderToUnityConsole;
        RunMode m_runMode = RunMode.Background;

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

        [MenuItem(menuItem_OpenInBlender, priority = 0)]
        static void OpenBlender()
        {
            var selectedAsset = Selection.objects[0] as GameObject;
            if (selectedAsset == null)
                return;

            var server = MeshSyncMenu.CreateMeshSyncServerMenu(null);

            server.m_DCCAsset = selectedAsset;
            server.m_DCCInterop = MeshSyncServerInspector.GetLauncherForAsset(selectedAsset);

            server.m_DCCInterop.OpenDCCTool(selectedAsset);
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

        #region IDCCLauncher

        public void OpenDCCTool(GameObject asset)
        {
            Cleanup();

            var startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.FileName = GetBlenderPath();

            var assetPath = AssetDatabase.GetAssetPath(asset).Replace("Assets/", string.Empty);
            var absoluteAssetPath = Path.Combine(Application.dataPath, assetPath).Replace('\\', '/');

            var scriptPath = Path.GetFullPath("Packages/com.unity.meshsync/Editor/Scripts/DCCIntegration/Launchers/blender_interop.py");

            startInfo.Arguments = $"\"{absoluteAssetPath}\" -P \"{scriptPath}\"";

            //startInfo.Arguments = $"-P \"{s_ScriptPath}\" -- {port}";
            if (m_runMode != RunMode.GUI)
                startInfo.Arguments = "-b " + startInfo.Arguments;

            if (m_runMode == RunMode.Background)
            {
                startInfo.UseShellExecute = false;
                startInfo.CreateNoWindow = true;
                startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            }

            m_blenderProcess = new System.Diagnostics.Process();

            if (m_redirectBlenderToUnityConsole)
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
            m_blenderProcess.Start();

            if (m_redirectBlenderToUnityConsole)
            {
                m_blenderProcess.BeginOutputReadLine();
                m_blenderProcess.BeginErrorReadLine();
            }
        }

        public void DrawDCCToolVersion(BaseMeshSync player)
        {
            var styleFold = EditorStyles.foldout;
            styleFold.fontStyle = FontStyle.Bold;
            player.foldBlenderSettings = EditorGUILayout.Foldout(player.foldBlenderSettings, "Blender settings", true, styleFold);

            if (player.foldBlenderSettings)
            {
                GUILayout.BeginVertical("Box");

                var blenderPath = GetBlenderPath();

                var newBlenderPath = EditorGUILayout.TextField("Blender path:", blenderPath);
                if (newBlenderPath != blenderPath)
                {
                    SetBlenderPath(newBlenderPath);
                    OpenBlender();
                }

                if (player is MeshSyncServer server)
                {
                    if (m_blenderProcess == null)
                    {
                        if (GUILayout.Button("Open Blender"))
                        {
                            server.m_DCCInterop.OpenDCCTool(server.m_DCCAsset);
                        }
                    }
                    else if (GUILayout.Button("Restart Blender"))
                    {
                        server.m_DCCInterop.OpenDCCTool(server.m_DCCAsset);
                    }

                    var runMode = (RunMode)EditorGUILayout.EnumPopup("Run mode:", m_runMode);
                    if (runMode != m_runMode)
                    {
                        m_runMode = runMode;
                        if (m_blenderProcess != null)
                        {
                            OpenDCCTool(server.m_DCCAsset);
                        }
                    }

                    var newRedirect = EditorGUILayout.Toggle("Redirect Blender to Unity Console:", m_redirectBlenderToUnityConsole);
                    if (newRedirect != m_redirectBlenderToUnityConsole)
                    {
                        m_redirectBlenderToUnityConsole = newRedirect;
                        if (m_blenderProcess != null)
                        {
                            OpenDCCTool(server.m_DCCAsset);
                        }
                    }
                }

                GUILayout.EndVertical();
            }

            EditorGUILayout.Space();
        }

        public void Cleanup()
        {
            if (m_blenderProcess != null)
            {
                try
                {
                    m_blenderProcess.Kill();
                }
                catch { }

                m_blenderProcess = null;
            }
        }

        #endregion // IDCCLauncher
    }
}

