﻿using System.Collections;
using NUnit.Framework;
using Unity.FilmInternalUtilities.Editor;
using UnityEngine.TestTools;
using UnityEditor.PackageManager;
using UnityEditor.PackageManager.Requests;
using UnityEngine;

namespace Unity.MeshSync.Editor.Tests {
internal class PluginTest {
    
    [UnityTest]
    public IEnumerator CheckPluginVersion() {

        ListRequest list = Client.List(true, false);
        while (!list.IsCompleted)
            yield return null;

        string pluginVersion   = Lib.GetPluginVersion();
        bool   parsed          = TryParseMajorAndMinorVersion(pluginVersion, out Vector2Int libVersion);
        int    libMajorVersion = libVersion.x;
        int    libMinorVersion = libVersion.y;
        Assert.IsTrue(parsed, $"Invalid version: {pluginVersion}");
        
        
        foreach (PackageInfo packageInfo in list.Result) {
            if (packageInfo.name != MeshSyncConstants.PACKAGE_NAME)
                continue;

            parsed = PackageVersion.TryParse(packageInfo.version, out PackageVersion packageVersion);
            Assert.IsTrue(parsed);
            
            
            //Based on our rule to increase the major/minor version whenever we change any plugin code,
            //it's ok for the patch version to be different.
            Assert.AreEqual(libMajorVersion, packageVersion.Major, $"Major: {libMajorVersion} !={packageVersion.Major}");           
            Assert.AreEqual(libMinorVersion, packageVersion.Minor, $"Minor: {libMinorVersion} !={packageVersion.Minor}");            
            yield break;
        }
        
    }
    
//----------------------------------------------------------------------------------------------------------------------
    
    //Parse only the major and minor version of semantic versioning
    public static bool TryParseMajorAndMinorVersion(string semanticVer, out Vector2Int version) {
        version = Vector2Int.zero;
        string[] tokens = semanticVer.Split('.');
        if (tokens.Length <= 2)
            return false;

        if (!int.TryParse(tokens[0], out int major))
            return false;

        if (!int.TryParse(tokens[1], out int minor))
            return false;

        version = new Vector2Int(major, minor);
        return true;
    } 
    
}

} //end namespace
