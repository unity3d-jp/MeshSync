using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;
using UnityEngine;

namespace Unity.MeshSync.Editor
{
    internal static class BlenderInterop
    {
        enum RunMode
        {
            Background,
            Console,
            GUI
        }

        const string editorSettingPath = "MESHSYNC_BLENDER_PATH";

        static Process blenderProcess;

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

        [MenuItem("Assets/MeshSync/Open in blender", priority = 0)]
        static void OpenPreview()
        {
            var selectedAsset = Selection.objects[0] as GameObject;
            if (selectedAsset == null)
                return;

            MeshSyncMenu.CreateMeshSyncServerMenu(null);

            var mode = RunMode.GUI;


            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.FileName = GetBlenderPath();

            var assetPath = AssetDatabase.GetAssetPath(selectedAsset).Replace("Assets/", string.Empty);
            var absoluteAssetPath = Path.Combine(Application.dataPath, assetPath).Replace('\\', '/');

            var scriptPath = Path.GetFullPath("Packages/com.unity.meshsync/Editor/Scripts/blender_interop.py");

            startInfo.Arguments = $"\"{absoluteAssetPath}\" -P \"{scriptPath}\"";

            //startInfo.Arguments = $"-P \"{s_ScriptPath}\" -- {port}";
            if (mode != RunMode.GUI)
                startInfo.Arguments = "-b " + startInfo.Arguments;

            if (mode == RunMode.Background)
            {
                startInfo.UseShellExecute = false;
                startInfo.CreateNoWindow = true;
                startInfo.WindowStyle = ProcessWindowStyle.Hidden;
            }

            blenderProcess = new Process();

            //if (redirectBlenderToUnityConsole)
            //{
            //    startInfo.RedirectStandardOutput = true;
            //    startInfo.RedirectStandardError = true;

            //    m_Process.ErrorDataReceived -= M_ProcessOnErrorDataReceived;
            //    m_Process.OutputDataReceived -= M_ProcessOnOutputDataReceived;

            //    m_Process.ErrorDataReceived += M_ProcessOnErrorDataReceived;
            //    m_Process.OutputDataReceived += M_ProcessOnOutputDataReceived;

            //    startInfo.UseShellExecute = false;
            //}

            blenderProcess.StartInfo = startInfo;
            blenderProcess.Start();

            //if (redirectBlenderToUnityConsole)
            //{
            //    m_Process.BeginOutputReadLine();
            //    m_Process.BeginErrorReadLine();
            //}

            //Connection = new BlenderConnection(port);
        }

        [MenuItem("Assets/MeshSync/Open in blender", validate = true, priority = 0)]
        static bool CanOpenPreview()
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
    }
}
