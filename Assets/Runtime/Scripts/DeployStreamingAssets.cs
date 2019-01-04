#if UNITY_EDITOR
using System;
using System.IO;
using System.Text.RegularExpressions;
using UnityEngine;
using UnityEditor;

namespace UTJ.MeshSync
{
    public static class DeployStreamingAssets
    {
        const string AssetDirName = "MeshSyncServerRoot";

        public static bool Deploy(bool overwrite = false)
        {
            try
            {
                string thisFile = new System.Diagnostics.StackTrace(true).GetFrame(0).GetFileName().Replace('\\', '/');
                var match = new Regex(@"^(.+?Assets)/Runtime").Match(thisFile);
                if (match.Success)
                {
                    var srcPath = match.Groups[1].Value + "/StreamingAssets/" + AssetDirName;
                    var dstPath = Application.streamingAssetsPath + "/" + AssetDirName;
                    if (!Directory.Exists(dstPath))
                        return CopyDirectory(srcPath, dstPath, overwrite);
                    return true;
                }
            }
            catch (Exception e)
            {
                Debug.LogWarning(e);
            }
            return false;
        }

        static bool CopyDirectory(string src, string dst, bool overwrite = false)
        {
            var dir = new DirectoryInfo(src);
            if (!dir.Exists)
                return false;
            if (!Directory.Exists(dst))
                Directory.CreateDirectory(dst);

            var isMeta = new Regex(@"\.meta$");
            foreach (var file in dir.GetFiles())
            {
                if (isMeta.Match(file.Name).Success)
                    continue; // ignore .meta

                var path = Path.Combine(dst, file.Name);
                file.CopyTo(path, overwrite);

                // ImportAsset() require relative path from Assets/
                var importPath = new Regex(@".+?/Assets/").Replace(path, "Assets/");
                AssetDatabase.ImportAsset(importPath);
            }

            // recurse
            foreach (var subdir in dir.GetDirectories())
                CopyDirectory(subdir.FullName, Path.Combine(dst, subdir.Name), overwrite);

            return true;
        }

    }
}
#endif
