using System;
using System.IO;
using System.Security.Cryptography;
using UnityEngine;

namespace Unity.AnimeToolbox {

//[TODO-sin: 2020-5-7] Move this to AnimeToolbox
internal static class FileUtility {

    internal static string ComputeMD5(string path) {
        using (var md5 = MD5.Create()) {
            using (var stream = File.OpenRead(path)) {
                byte[] hash = md5.ComputeHash(stream);
                string str = BitConverter.ToString(hash).Replace("-", "").ToLowerInvariant();
                return str;
            }
        }
    }
    
//----------------------------------------------------------------------------------------------------------------------

    #region json
    internal static T DeserializeFromJson<T>(string path) {
        string json = File.ReadAllText(path);
        return JsonUtility.FromJson<T>(json);
    }
    
    internal static void SerializeToJson<T>(T obj, string path, bool prettyPrint=false) {
        string json = JsonUtility.ToJson(obj, prettyPrint);
        File.WriteAllText(path, json);
    }
    #endregion
    
    
//---------------------------------------------------------------------------------------------------------------------

    internal static bool DeleteFilesAndFolders(string path) {
        return FileUtility.DeleteFilesAndFolders(new DirectoryInfo(path));        
    }

//---------------------------------------------------------------------------------------------------------------------

    internal static bool DeleteFilesAndFolders(DirectoryInfo di) {
        //Try to delete the internal contents of the directory 
        try {
            foreach (FileInfo file in di.EnumerateFiles()) {
                file.Delete(); 
            }
            foreach (DirectoryInfo dir in di.EnumerateDirectories()) {
                dir.Delete(true); 
            }
        } catch {
            Debug.LogError("Error when trying to delete: " + di.FullName);
            return false;
        }        

        //Try delete the directory itself at the end.
        try {
            di.Delete(true);
        } catch {
        }        
        return true;
    }    
    
//----------------------------------------------------------------------------------------------------------------------
    internal static void CopyRecursive(string sourceDir, string targetDir, bool overwrite)
    {
        System.IO.Directory.CreateDirectory(targetDir);

        DirectoryInfo sourceDI = new DirectoryInfo(sourceDir);
        foreach (FileInfo file in sourceDI.EnumerateFiles()) {
            File.Copy(file.FullName, Path.Combine(targetDir, file.Name), overwrite);
        }
        foreach (DirectoryInfo dir in sourceDI.EnumerateDirectories()) {
            CopyRecursive(dir.FullName, Path.Combine(targetDir, dir.Name), overwrite);
        }

    }    
    
}

}