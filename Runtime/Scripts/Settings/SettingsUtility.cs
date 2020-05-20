using System.Collections;
using System.Collections.Generic;
using UnityEngine;

internal class SettingsUtility 
{
    internal static void Load() {
        
    }

    internal static void Save() {
        
    }
    
    // internal virtual void Save(bool saveAsText)
    // {
    //     if (s_Instance == null)
    //     {
    //         Debug.Log("Cannot save ScriptableSingleton: no instance!");
    //         return;
    //     }
    //     string filePath = GetFilePath();
    //     if (!string.IsNullOrEmpty(filePath))
    //     {
    //         string directoryName = Path.GetDirectoryName(filePath);
    //         if (!Directory.Exists(directoryName))
    //         {
    //             Directory.CreateDirectory(directoryName);
    //         }
    //         System.IO.File.WriteAllText(filePath, EditorJsonUtility.ToJson(s_Instance, true));
    //     }
    // }
    //
    // private static string GetFilePath()
    // {
    //     foreach(var attr in typeof(T).GetCustomAttributes(true)) {
    //         FilePathAttribute filePathAttribute = attr as FilePathAttribute;
    //         if (filePathAttribute != null)
    //         {
    //             return filePathAttribute.filepath;
    //         }
    //     }
    //     return null;
    // }
}
