#if UNITY_EDITOR
using System.IO;
using System.Text.RegularExpressions;
using UnityEngine;
using UnityEditor;

namespace Unity.MeshSync {
    internal static class DeployStreamingAssets  {

        internal const string SERVER_ROOT_DIR_NAME = "MeshSyncServerRoot";

        internal static bool Deploy(bool overwrite = false) {
            string serverRootSrcDir = Path.Combine("Packages",MeshSyncConstants.PACKAGE_NAME,"Editor",SERVER_ROOT_DIR_NAME);

            string srcPath = Path.GetFullPath(serverRootSrcDir);
            if (!Directory.Exists(srcPath))
                return false;
            
            string dstPath = Path.Combine(Application.streamingAssetsPath,SERVER_ROOT_DIR_NAME);
            return CopyDirectory(srcPath, dstPath, overwrite);
        }
        
//----------------------------------------------------------------------------------------------------------------------                

        static bool CopyDirectory(string src, string dst, bool overwrite = false)
        {
            DirectoryInfo dir = new DirectoryInfo(src);
            if (!dir.Exists)
                return false;
            
            Directory.CreateDirectory(dst);

            Regex isMeta = new Regex(@"\.meta$");
            foreach (FileInfo file in dir.GetFiles()) {
                if (isMeta.Match(file.Name).Success)
                    continue; // ignore .meta
                
                string destPath = Path.Combine(dst, file.Name);
                
                //Skip existing files if overwrite is false
                if (!overwrite && File.Exists(destPath))
                    continue;
                
                file.CopyTo(destPath, overwrite);

                // ImportAsset() require relative path from Assets/
                string importPath = AssetUtility.NormalizeAssetPath(destPath);
                AssetDatabase.ImportAsset(importPath);
            }

            // recurse
            foreach (DirectoryInfo subDir in dir.GetDirectories())
                CopyDirectory(subDir.FullName, Path.Combine(dst, subDir.Name), overwrite);

            return true;
        }
        

    }
}
#endif
